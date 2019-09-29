# Алгоритм планирования

Был реализован следующий алгоритм планирования: 
Каждый worker имеет свою локальную очередь (это могла быть MutexQueue, реализованная через std::mutex и std::queue, или LFABQueue, реализованная на массиве с индексами на голову и хвост очереди, из-за чего реализация получилась lock-free, но с ограничением на размер очереди). \
Также была глобальная очередь MutexQueue. \
Каждый worker при поиске нового fiber'а каждый 61ый раз ищет fiber сначала в глобальной очереди - это сделано для того, чтобы fiber'ы не простаивали в глобальной очереди, так как без этого каждый worker возьмет по одной задаче и будет выполнять ее, пока она не выполнится, и только потом возьмет следующую из глобальной очереди, что также нарушает условие "честности". Если worker не искал или не нашел в глобальной очереди, то после этого ищет fiber в локальной очереди. Если же не нашел и там, то идет в глобальную очередь (возможно еще раз). Если не было и там, то берет случайно другого worker'а и пытается украть fiber у него. \
Когда fiber кладется на выполнение, то проверяется, делает это worker или кто-то другой. В первом случае fiber кладется в локальную очередь, но в случае ее переполнения часть задач перекладывается в глобальную очередь. Во втором случае fiber сразу кладется в глобальную очередь. \
 \
Отдельно был написан алгоритм планирования GlobalFIFO, у которого была одна глобальная очередь с std::mutex, который использовался в качестве baseline.

# Вариации алгоритма
Были проверены несколько эвристик: \
В случае MutexQueue проверялось следующее:
 - Лучше красть у другого worker'а один fiber или половину (StealOne/StealHalf)
 - Лучше брать из глобальной очереди один fiber или сразу все (GetOneFromGlob/GetAllFromGlob)

В случае LFABQueue проверялось следующее:
 - В случае переполнения локальной очереди класть в глобальную очередь один fiber или половину (PutOneToGlob/PutHalfToGlob)
 - Лучше красть у другого worker'а один fiber или половину (StealOne/StealHalf)

Заметим, что тест `test_scheduler.cpp:SchedulerTest/YieldAndFairness` показывает, что в случае одной нити все алгоритмы планирования работают "честно", то есть планируют задачи равномерно.
# Бенчмарки

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

---------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                          Time             CPU   Iterations
---------------------------------------------------------------------------------------------------------------------------------------
BM_SlowThread<algo::GlobalFIFO>/4/1000/10/100                                                    125 ms         3.47 ms          100
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetAllFromGlob>/4/1000/10/100         124 ms         3.55 ms          100
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealOne_GetAllFromGlob>/4/1000/10/100          121 ms         3.47 ms          100
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealHalf_GetOneFromGlob>/4/1000/10/100         122 ms         3.57 ms          100
BM_SlowThread<WorkStealing_MutexQueue_MutexQueue_StealOne_GetOneFromGlob>/4/1000/10/100          119 ms         3.44 ms          100
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealHalf>/4/1000/10/100           120 ms         3.48 ms          100
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealHalf>/4/1000/10/100            121 ms         3.56 ms          100
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutHalfToGlob_StealOne>/4/1000/10/100            125 ms         3.61 ms          100
BM_SlowThread<WorkStealing_MutexQueue_LFABQueue_PutOneToGlob_StealOne>/4/1000/10/100             126 ms         3.80 ms          100

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

Из которых видно, что реализация WorkStealing алгоритма планирования с lock-free локальной очередью и кражей половины fiber'ов работают быстрее.

# Выводы
WorkStealing алгоритм оказался быстрее, чем GlobalFIFO, потому что в нем реже происходит обращение к глобальной очереди, а следовательно происходит меньше системных вызовов (lock и unlock на mutex'е). Также в GlobalFIFO, пока один поток работает с глобальной очередью, другие обязаны ждать его, а в WorkStealing алгоритме каждый поток в основном работает со своей локальной очередью. \
Lock-free реализация очереди работает быстрее по той причине, что во время работы с очередью не проиходить системных вызовов, а также нет блокировки на работу с очередью, то есть несколько потоков могут одновременно работать с одной и той же очередью (пока один поток крадет задачи, другой может спокойно класть или брать задачи из очереди). \
Кража одного fiber'а хуже, потому что остается большая разница в кол-ве fiber'ов, в то время, как worker'ы равны между собой. При краже одного fiber'а worker выполняет его, пока тот не завершится (что, кстати, еще и нарушает "честность"). Если же fiber не создал новых fiber'ов, то worker обязан опять идти и красть задачи. В случае, когда worker крадет сразу половину fiber'ов, то он не пойдет красть новые fiber'ы, пока не выполнит все существующие, а следовательно получается меньше лишних взаимодействий между worker'ами. \
На некоторых бенчмарках быстрее работает реализация, в которой в глобальную очередь возвращается половина задач (`BM_SingleSpawner` и `BM_MergeSort`). Это происходит из-за того, что в этих бенчмарках создается большое количество fiber'ов  локально, то есть новые fiber'ы кладутся в локальную очередь, и в случае `PutOneToGlob` worker/worker'ы вынуждены часто перекладывать fiber'ы в глобальную очередь, защищенную одним мьютексом. 
