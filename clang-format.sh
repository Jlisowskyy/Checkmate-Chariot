#!/bin/sh
clang-format -style=file -i $(find . -regex '.*\.\(cpp\|cc\|h\|c\)')