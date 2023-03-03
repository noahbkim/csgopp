#include <memory>
#include <utility>
#include <iostream>
#include <fstream>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <google/protobuf/io/coded_stream.h>

#include "csgopy.h"
#include "csgopp/client.h"

using namespace nanobind::literals;

using google::protobuf::io::IstreamInputStream;
using google::protobuf::io::CodedInputStream;
using csgopp::client::Client;

class PythonAdapter : public csgopp::client::ClientObserverBase<PythonAdapter>
{
public:
    explicit PythonAdapter(Client& client, nanobind::object observer)
        : ClientObserverBase(client)
        , observer(std::move(observer))
    {}

    void on_entity_update(
        csgopp::client::ClientObserverBase<PythonAdapter>::Client &client,
        const csgopp::client::entity::Entity *entity,
        const std::vector<uint16_t> &indices
    ) override
    {
        auto callback = this->observer.attr("on_entity_update");
        if (callback.is_valid())
        {
            callback(indices);
        }
    }

    void debug() const
    {
        nanobind::print(this->observer);
    }

private:
    nanobind::object observer;
};

void print(const std::string& string)
{
    std::cout << string << std::endl;
}

void parse(const std::string& path, nanobind::object observer)
{
    std::ifstream file_stream(path, std::ios::binary);
    IstreamInputStream file_input_stream(&file_stream);
    CodedInputStream coded_input_stream(&file_input_stream);

    Client<PythonAdapter> client(coded_input_stream, observer);
    while (client.advance(coded_input_stream));
}

NB_MODULE(csgopy, mod) {
    nanobind::exception<csgopp::client::GameError>(mod, "GameError");
    mod.def("parse", &parse);
    mod.def("print", &print);
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
