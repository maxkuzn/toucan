# Алгоритм планирования

Были реализованы следующий алгоритм планирования: 
Каждый worker имел свою локальную очередь (это могла быть MutexQueue, реализованная через std::mutex и std::queue, или LFABQueue, реализованная на массиве с индексами на голову и хвост очереди, из-за чего реализация получилась lock-free, но с ограничением на размер очереди). \
Также была глобальная очередь MutexQueue. \
Каждый worker при поиске нового fiber'а каждый 61ый раз искал файбер сначала в глобальной очереди. Если не искал или не находил, то после этого искал fiber в локальной очереди. Если же не находил и там, то шел в глобальную очередь (возможно еще раз). Если не было и там, то брал случайно другого worker'а и пытался украть fiber у него. \
Когда fiber клали на выполнение, то проверялось, делал это worker или кто-то другое. В первом случае fiber клался в локальную очередь, но в случае ее переполнения часть задач клалось в глобальную очередь. Во втором случае fiber сразу клался в глобальную очередь. \
 \
Отдельно был написан алгоритм планирования GlobalFIFO, у которого была одна глобальная очередь с std::mutex, который использовался в качестве baseline. \
 \
Были проверены несколько эвристик: \
В случае MutexQueue проверялось следующее:
 - Лучше красть у другого worker'а один fiber или половину (StealOne/StealHalf)
 - Лучше брать из глобальной очереди один fiber или сразу все (GetOneFromGlob/GetAllFromGlob)

В случае LFABQueue проверялось следующее:
 - В случае переполнения локальной очереди класть в глобальную очередь один fiber или половину (PutOneToGlob/PutHalfToGlob)
 - Лучше красть у другого worker'а один fiber или половину (StealOne/StealHalf)

Заметим, что тест `test_scheduler.cpp:SchedulerTest/YieldAndFairness` показывает, что в случае одной нити все алгоритмы планирования работают "честно", то есть планируют задачи равномерно. \
 \
Было сравнение производительности. Вот некоторые результаты бенчмарков:
```
-----------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                         Time             CPU   Iterations
-----------------------------------------------------------------------------------------------------------------------------------
BM_SingleSpawner<algo::GlobalFIFO>/4/1000/10/0                                                  164 ms        0.040 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetAllFromGlob>/4/1000/10/0       162 ms        0.040 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_MutexQueue_StealOne_GetAllFromGlob>/4/1000/10/0        160 ms        0.039 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetOneFromGlob>/4/1000/10/0       164 ms        0.041 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_MutexQueue_StealOne_GetOneFromGlob>/4/1000/10/0        145 ms        0.038 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealHalf>/4/1000/10/0         141 ms        0.036 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealHalf>/4/1000/10/0          148 ms        0.038 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealOne>/4/1000/10/0          161 ms        0.041 ms          100
BM_SingleSpawner<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealOne>/4/1000/10/0           165 ms        0.041 ms          100

-----------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                         Time             CPU   Iterations
-----------------------------------------------------------------------------------------------------------------------------------
BM_SlowThread<algo::GlobalFIFO>/4/1000/10/100                                                   136 ms         4.00 ms          100
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetAllFromGlob>/4/1000/10/100        140 ms         4.36 ms          100
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealOne_GetAllFromGlob>/4/1000/10/100         155 ms         4.97 ms          100
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetOneFromGlob>/4/1000/10/100        144 ms         4.62 ms          137
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealOne_GetOneFromGlob>/4/1000/10/100         134 ms         4.43 ms          130
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealHalf>/4/1000/10/100          152 ms         5.04 ms          136
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealHalf>/4/1000/10/100           125 ms         3.95 ms          100
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealOne>/4/1000/10/100           160 ms         5.28 ms          100
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealOne>/4/1000/10/100            154 ms         5.29 ms          114

------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                          Time             CPU   Iterations
------------------------------------------------------------------------------------------------------------------------------------
BM_MergeSort<algo::GlobalFIFO>/4/1024                                                           27.6 ms        0.032 ms         1000
BM_MergeSort<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetAllFromGlob>/4/1024                29.7 ms        0.030 ms          100
BM_MergeSort<WorkStealing_MutexQueue_MutexQueue_StealOne_GetAllFromGlob>/4/1024                 29.6 ms        0.030 ms          100
BM_MergeSort<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetOneFromGlob>/4/1024                28.2 ms        0.028 ms          100
BM_MergeSort<WorkStealing_MutexQueue_MutexQueue_StealOne_GetOneFromGlob>/4/1024                 28.0 ms        0.030 ms          100
BM_MergeSort<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealHalf>/4/1024                  25.4 ms        0.027 ms          100
BM_MergeSort<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealHalf>/4/1024                   26.9 ms        0.028 ms          100
BM_MergeSort<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealOne>/4/1024                   28.1 ms        0.029 ms          100
BM_MergeSort<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealOne>/4/1024                    27.8 ms        0.029 ms          100

--------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                            Time             CPU   Iterations 
--------------------------------------------------------------------------------------------------------------------------------------
BM_DifferentSpawners<algo::GlobalFIFO>/8/10000/100/10                                              719 ms        0.012 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetAllFromGlob>/8/10000/100/10   660 ms        0.011 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_MutexQueue_StealOne_GetAllFromGlob>/8/10000/100/10    658 ms        0.011 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetOneFromGlob>/8/10000/100/10   664 ms        0.013 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_MutexQueue_StealOne_GetOneFromGlob>/8/10000/100/10    664 ms        0.011 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealHalf>/8/10000/100/10     654 ms        0.011 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealHalf>/8/10000/100/10      650 ms        0.011 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealOne>/8/10000/100/10      651 ms        0.010 ms           10
BM_DifferentSpawners<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealOne>/8/10000/100/10       659 ms        0.011 ms           10
```

Из которых видно, что реализация WorkStealing алгоритма планирования, в котором worker'ы крадут половину задач, работает быстрее всех остальных. Тем не менее, в зависимости от бенчмарка, быстрее работает либо реализация, в которой в глобальную очередь возвращается половина задач (`BM_SingleSpawner` и `BM_MergeSort`), либо реализация, в которой в глобальную очередь возвращается только одна задача (`BM_SlowThread` и `BM_DifferentSpawners`).
