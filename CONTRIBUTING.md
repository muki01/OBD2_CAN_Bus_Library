# Contributing to the OBD2 CAN Bus Library

Thank you for taking the time to contribute! Every vehicle test, bug report, fix and idea makes this library better for the whole car-hacking and maker community.

By participating, you agree to follow the [Code of Conduct](CODE_OF_CONDUCT.md).

## Ways to Contribute

| | |
|---|---|
| 🚗 **Report a tested vehicle** | Tell us which cars work (or don't). Use the **Vehicle report** issue template. |
| 🐛 **Report a bug** | Use the **Bug report** template and include the debug output whenever possible. |
| 💡 **Suggest a feature** | Use the **Feature request** template. |
| 🔧 **Submit code** | Protocol fixes, new PIDs, new examples. |
| 📝 **Improve the docs** | Clearer explanations, wiring photos, corrections. |

## Reporting Bugs

Before opening an issue, please search the [existing issues](https://github.com/muki01/OBD2_CAN_Bus_Library/issues). A good report includes:

- The library version or commit
- The ESP32 board and the Arduino-ESP32 core version
- The CAN transceiver (TJA1050, SN65HVD230, …)
- The vehicle: make, model, year and engine
- The selected protocol and the protocol that was detected
- **The debug output** from `setDebug()`. The frames sent and received are the most useful information.

## Development Workflow

1. **Fork** the repository and create a branch:
   ```bash
   git checkout -b feature/my-improvement
   ```
2. Make your changes, keeping them **focused**: one fix or feature per pull request.
3. **Test on a real vehicle** when your change touches the frame handling or the connection logic, and say in the pull request which vehicle and protocol you tested on.
4. Make sure everything still **compiles** for the boards it supports.
5. Commit with a clear message.
6. Push and open a **pull request**, filling in the template.

## Coding Guidelines

- Follow the existing style of the file you are editing: naming, indentation and comment density.
- Keep debug strings in flash with `F()`.
- New public methods need an entry in the API tables of the README.

## License

By contributing, you agree that your contributions are licensed under the [GNU General Public License v3.0](LICENSE), and that the author may also offer them under a commercial license.
