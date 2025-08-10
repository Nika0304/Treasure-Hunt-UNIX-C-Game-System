# 🗺️ Treasure Hunt – UNIX C Game System

A **UNIX-based treasure hunt simulation** developed in **C**, split into three phases that combine file handling, process management, signals, and inter-process communication.

---

## 📌 Overview

This project simulates a digital treasure hunt game.  
It allows users to:
- Manage hunt sessions and treasures via file operations
- Interact with a live monitoring process through UNIX signals
- Compute player scores using external programs and pipes

💡 Developed as part of the **System Programming** lab at *Politehnica University of Timișoara*.

---

## 🧱 Components

| Phase | Component           | Description |
|-------|--------------------|-------------|
| 1     | `treasure_manager` | CLI tool to create/manage hunts & treasures via system calls |
| 2     | `treasure_hub`     | Interactive shell communicating with a monitor process using `sigaction()` |
| 3     | `score_calculator` | External tool computing per-user treasure values via pipe output |

---

## 🔧 Technologies & Concepts

- **Language & Tools**: C (POSIX), Bash, Make
- **System Calls**: `open()`, `read()`, `write()`, `lseek()`, `stat()`, `mkdir()`
- **Process Management**: `fork()`, `exec()`, `waitpid()`
- **Signals**: `sigaction()`, `kill()`
- **IPC**: Pipes
- **Data Handling**: File I/O, binary data formats

---

## 🎮 Features by Phase

- **📁 Phase 1 – File System**  
  Create and manage treasure hunt data using low-level file system calls.
  
- **🧠 Phase 2 – Signals & Process Control**  
  Real-time monitoring and control via signal handling.
  
- **🔁 Phase 3 – Pipes & Score Calculation**  
  Process player scores and treasure values using inter-process communication.

---


