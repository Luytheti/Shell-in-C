# 🐚 MyShell

A simple custom Unix-like shell implemented in C.  
This project demonstrates process creation, command execution, I/O redirection, sequential & parallel execution, and pipelining — mimicking core features of a Unix shell.

---

## ✨ Features
- Execute basic Linux commands using `fork()` + `execvp()`
- Built-in `cd` command to change directories
- Support for:
  - **Sequential commands** separated by `##`
  - **Parallel commands** separated by `&&`
  - **Input/Output redirection** using `<` and `>`
  - **Pipelining** using `|`
- Handles invalid commands gracefully
- Waits for child processes to finish (synchronization)
- Easy to extend with more shell-like features
