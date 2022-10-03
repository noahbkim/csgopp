#pragma once

#define GET(NAME, ...) [[nodiscard]] auto __VA_ARGS__ NAME() const { return this->_##NAME; }
