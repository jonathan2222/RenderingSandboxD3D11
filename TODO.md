# TODO

## Refactoring/Abstractions

- [ ] Separate the implementation code of the **`Load`** functions from **`ResourceManager`** into a separate class.

- [ ] Refactor rendering commands, maybe add a **Context** class for the separate rendering stages.

- [ ] Abstract the D3D11 code into its own classes.

## Fixes

- [ ] Fix problems with the **`ResourceInspector`** where it might display text when it should not. Make it also more robust to errors when some resources or parts of them are missing.

## New stuff

- [ ] Implement **IBL** in the **`PBRScene`** class.

- [ ] Add a why of logging only once even when the function is called multiple times.

- [ ] Add option to render text with the **`DebugRenderer`**.
