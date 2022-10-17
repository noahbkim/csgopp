#include <memory>
#include <utility>

#include "csgopy.h"
#include "csgopp/client.h"

using namespace nanobind::literals;

class PythonObserver : csgopp::game::SimulationObserverBase<PythonObserver>
{
public:
    explicit PythonObserver(nanobind::object observer) : observer(std::move(observer)) {}

    void debug() const
    {
        nanobind::print(this->observer);
    }

private:
    nanobind::object observer;
};

NB_MODULE(csgopy, mod) {
    nanobind::class_<PythonObserver>(mod, "PythonObserver")
        .def(nanobind::init<nanobind::object>())
        .def("debug", &PythonObserver::debug);
}

/*
import inspect
import functools

class Simulation:
    i = 0

    def __init__(self, observer):
        self.observer = observer

    def step(self):
        print("args", len(inspect.signature(self.observer.on_event).parameters))
        on_event = self.observer.on_event(self)
        next(on_event)

        self.i += 1

        try:
            on_event.send(self.i)
        except StopIteration:
            pass
        else:
            raise RuntimeError("iteration should be complete!")


def test(self, simulation):
    print("before", simulation.i)
    after = yield
    print("after", after, simulation.i)


class Observer:
    def _on_event(self, simulation: Simulation) -> None:
        print("before", simulation.i)
        after = yield
        print("after", after, simulation.i)

    on_event = _on_event


s = Simulation(Observer())
s.step()

print(inspect.isgeneratorfunction(Observer.on_event))
 */
