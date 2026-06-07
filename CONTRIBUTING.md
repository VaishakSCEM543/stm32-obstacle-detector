# Contributing to the STM32 Multi-Sensor Project

We welcome contributions to improve the embedded system!

## Getting Started
1. Fork the repository
2. Import the project into STM32CubeIDE
3. Create a new branch (`git checkout -b feature/your-feature`)

## Code Guidelines
- Keep all custom code between the `/* USER CODE BEGIN */` and `/* USER CODE END */` blocks in `main.c` so STM32CubeMX regeneration doesn't overwrite it.
- Use the `.clang-format` profile provided to format your C code.
- Avoid introducing blocking delays (`HAL_Delay`) during the main loop; use non-blocking `HAL_GetTick()` or the DWT timer.

## Pull Requests
1. Test your code on the physical STM32 Nucleo hardware if possible.
2. Ensure the GitHub Actions CI workflow passes.
3. Submit a Pull Request describing your changes.
