# MyShell

A simple custom Unix-like shell implemented in C.  
This project demonstrates process creation, command execution, I/O redirection, and support for sequential & parallel execution of commands.

---

## ✨ Features
- Execute basic Linux commands using `fork()` + `execvp()`
- Built-in `cd` command to change directories
- Support for:
  - **Sequential commands** separated by `##`
  - **Parallel commands** separated by `&&`
- Handles invalid commands gracefully
- Waits for child processes to finish (synchronization)
- Easy to extend with more shell-like features
