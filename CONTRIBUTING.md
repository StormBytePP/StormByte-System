# Contributing to StormByte-System

Issues and pull requests belong on **this** repository only. Fork and open a PR against `master`.

Coding rules for this repository are in [CODING_STYLE.md](CODING_STYLE.md). That file is the StormByte flavor: C++26, tabs, K&R functions, Doxygen on every member except `= delete`, visibility on the function not the return type, Conventional Commits in English. Read it before you write the patch.

By submitting a contribution you assign copyright in that contribution to the copyright holder of this repository (David C. Manuelda). The dual license in [LICENSE](LICENSE) can then apply to it.

Send only code you wrote and are free to assign. Do not send code owned by an employer, a third party, or another project unless you already have the right to assign that copyright here. After the contribution lands, that copyright sits with the copyright holder of this repository. Each contributor is responsible for that clearance. The project does not audit origin and does not take on that liability.

If a contribution was sent in error (you could not assign it, or a third party owns it), the copyright holder of this repository may revert the associated commit or commits. The true rights holder of that material may also ask for that revert directly.

New `.hxx` / `.h` / `.hpp` / `.cxx` / `.cpp` / `.cc` / `.c` files must start with the license header used in this repository, unchanged. Do not invent a shorter banner. CMake, Markdown and other non-C++ files do not take that header.

## Pull requests

- Keep the diff on one topic. Do not mix license, features and drive-by renames.
- Follow [CODING_STYLE.md](CODING_STYLE.md) (language, visibility, Doxygen, commits, tests, file headers).
- Add or extend tests when you change behavior.
- Do not add files under `thirdparty/` in a feature PR.
- Maintainers may ask you to rebase on `master`.
