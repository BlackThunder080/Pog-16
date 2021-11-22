from cx_Freeze import setup, Executable

setup(name = "./build/assembler/assembler" ,
      version = "1.0" ,
      description = "" ,
      executables = [Executable("assembler.py")])
