from typing import TextIO, TypeVar, Any

T = TypeVar("T", bound=dict[str, Any])


def read(file: TextIO, skip_whitespace: bool = False) -> str:
    character = file.read(1)
    while skip_whitespace and character and character.isspace():
        character = file.read(1)
    return character


def parse_string(file: TextIO) -> str:
    result = ""
    escaped = False
    while True:
        character = read(file)
        if character == "\\":
            escaped = True
        elif character == "\"" and not escaped:
            return result
        result += character


def parse_object(file: TextIO) -> dict[str, str | T]:
    result = {}
    while True:
        character = read(file, skip_whitespace=True)
        if character == "}" or character == "":
            break
        assert character == "\""
        key = parse_string(file)
        value = parse_any(file)
        if key in result and isinstance(result[key], dict):
            result[key].update(value)
        else:
            result[key] = value
    return result


def parse_any(file: TextIO) -> str | dict[str, str | T]:
    character = read(file, skip_whitespace=True)
    if character == "{":
        return parse_object(file)
    elif character == "\"":
        return parse_string(file)
    raise ValueError(f"invalid character {character}")


def parse(file: TextIO) -> dict[str, str | T]:
    return parse_object(file)


def main():
    import sys
    import pprint

    with open(sys.argv[1]) as file:
        for key, value in parse(file)["items_game"]["items"].items():
            if key.isnumeric() and int(key) <= 64:
                print(f"case {key}: return \"{value['name']}\";")


if __name__ == "__main__":
    main()
