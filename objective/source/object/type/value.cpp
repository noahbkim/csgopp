#include "objective/type/value.h"

namespace objective::type
{

void ValueType::emit(code::Declaration& declaration, code::Declaration::Member& member) const
{
    member.type = this->represent();
}

void ValueType::emit(layout::Cursor& cursor) const
{
    cursor.note(this->represent());
}

}
