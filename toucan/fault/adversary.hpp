#pragma once

#include <memory>

namespace toucan {
namespace fault {

/////////////////////////////////////////////////////////////////////

class IAdversary {
  public:
    virtual ~IAdversary() = default;

    // Test

    virtual void Reset() = 0;
    virtual void PrintReport() = 0;

    // This thread

    virtual void Enter() = 0;

    // Inject fault (yield, sleep, park)
    virtual void Fault() = 0;

    // For lock-free and wait-free algorithms
    virtual void ReportProgress() = 0;

    virtual void Exit() = 0;
};

using IAdversaryPtr = std::shared_ptr<IAdversary>;

/////////////////////////////////////////////////////////////////////

// Not thread safe, should be externally synchronized

IAdversary* GetAdversary();
void SetAdversary(IAdversaryPtr adversary);

void AccessAdversary();

}  // namespace fault
}  // namespace toucan

