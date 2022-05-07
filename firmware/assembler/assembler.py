import sys
from csv import reader

global_registers = []
global_instruction_types = []

class Token:
    def __init__(self, tok):
        # Validate tokens
        self.token = tok

class Assembler:
    def __init__(self, asm_path_name):
        self.asm_path_name = asm_path_name
        self.tokens = []
        # Initialize register name information
        with open('arch-spec/registers.txt', 'r') as fd:
            csv_reader = reader(fd)
            self.registers = list(csv_reader)
            global_registers = self.registers

        # Initialize instruction type name information
        with open('arch-spec/instructions.txt', 'r') as fd:
            csv_reader = reader(fd)
            self.instruction_types = list(csv_reader)
            global_instruction_types = self.instruction_types
        
        # Read all lines from assembly file
        with open(self.asm_path_name, 'r') as fd:
            self.asm_lines = fd.readlines()
        
        # Remove empty lines
        while "\n" in self.asm_lines:
            self.asm_lines.remove("\n")
        
        # Remove comments
        for i in range(len(self.asm_lines)):
            split_comment = self.asm_lines[i].split("//", 1)
            self.asm_lines[i] = split_comment[0]
            self.asm_lines[i] = self.asm_lines[i].strip()

    def tokenize(self):
        token_builder = ""
        for line in self.asm_lines:
            for char in line:
                if char == ' ':
                    if len(token_builder) != 0:
                        self.tokens.append(token_builder)
                    token_builder = ""
                elif char == ',':
                    if len(token_builder) != 0:
                        self.tokens.append(token_builder)
                    self.tokens.append(",")
                    token_builder = ""
                elif char == '(':
                    if len(token_builder) != 0:
                        self.tokens.append(token_builder)
                    self.tokens.append("(")
                    token_builder = ""
                elif char == ')':
                    if len(token_builder) != 0:
                        self.tokens.append(token_builder)
                    token_builder = ""
                    self.tokens.append(")")
                else:
                    token_builder += char
            if len(token_builder) != 0:
                self.tokens.append(token_builder)
            token_builder = ""
        print(self.tokens)

    def parser(self):
        sections = [""]
        # Verify meaning of assembly
        # while len(self.tokens) != 0:
            # Query what line i
        

def main():
    # Check if assembly file in command line argumentss
    if len(sys.argv) != 2:
        print("Error: asm file missing")
        sys.exit()

    # Initialize register and instruction type information
    asm = Assembler(sys.argv[1])

    # Tokenize assembly file
    asm.tokenize()

    # Verify semantics
    # asm.parse()

if __name__ == "__main__":
    main()