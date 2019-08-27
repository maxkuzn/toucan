#include <toucan/fault/adversary.hpp>

/*
#include <twist/fault/yielder.hpp>

#include <twist/support/assert.hpp>
#include <twist/support/singleton.hpp>
#include <twist/support/sync_output.hpp>
#include <twist/logging/logging.hpp>
*/

#include <atomic>
#include <cstdint>
#include <iostream>
#include <thread>

namespace toucan {
namespace fault {

/////////////////////////////////////////////////////////////////////

class DumbAdversary : public IAdversary {
  public:
    void Reset() override {
    }

    void PrintReport() override {
    }

    void Enter() override {
    }

    void Fault() override {
    }

    void ReportProgress() override {
    }

    void Exit() override {
    }
};

/////////////////////////////////////////////////////////////////////

class Holder {
  public:
    Holder() : adversary_(CreateDefaultAdversary()) {
    }

    IAdversary* Get() {
        return adversary_.get();
    }

    void Set(IAdversaryPtr adversary) {
        adversary_ = std::move(adversary);
    }

  private:
    static IAdversaryPtr CreateDefaultAdversary() {
        return std::make_shared<DumbAdversary>();
    }

  private:
    IAdversaryPtr adversary_;
};

static Holder holder;

IAdversary* GetAdversary() {
    return holder.Get();
}

void SetAdversary(IAdversaryPtr adversary) {
    holder.Set(adversary);
}

void AccessAdversary() {
    (void)GetAdversary();
}

}  // namespace fault
}  // namespace toucan

