#include "object.h"

ConstantReferenceBinding::CasterMap ConstantReferenceBinding::casters{
    {&typeid(bool), &cast<bool>},
    {&typeid(uint32_t), &cast<uint32_t>},
    {&typeid(int32_t), &cast<int32_t>},
    {&typeid(float), &cast<float>},
    {&typeid(Vector2), &cast<Vector2>},
    {&typeid(Vector3), &cast<Vector3>},
    {&typeid(std::string), &cast<std::string>},
    {&typeid(uint64_t), &cast<uint64_t>},
    {&typeid(int64_t), &cast<int64_t>},
};

