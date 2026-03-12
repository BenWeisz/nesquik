from typing import List

def read_file(path: str) -> List[str]:
    """ Read in the files lines. """
    file_lines = []
    with open(path, "r") as in_file:
        file_lines = in_file.readlines()
        file_lines = [line.strip("\n") for line in file_lines]
    return file_lines

NUM_SPACES_TO_INDENT = 4
def add_slashes(file_lines: List[str]) -> List[str]:
    """ Add slashes to the lines. """

    # Determine the maximum number characters per line
    lines_num_indents = map(
        lambda x: int(
            (len(x) + NUM_SPACES_TO_INDENT - 1) / NUM_SPACES_TO_INDENT) + 1
        , file_lines)
    lines_max_num_indents = max(lines_num_indents)
    max_line_len = NUM_SPACES_TO_INDENT * lines_max_num_indents

    new_lines = []
    for line in file_lines:
        num_spaces_needed = max_line_len - len(line)
        new_line = line + (" " * num_spaces_needed) + "\\"
        new_lines.append(new_line)

    return new_lines

if __name__ == "__main__":
    lines = read_file("../include/list/list.h")
    lines = add_slashes(lines)

    for line in lines:
        print(line)