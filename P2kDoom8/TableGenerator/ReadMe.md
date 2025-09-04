# Table Generator

Utility for generating lookup tables and constants used in graphics and rendering **[Doom8088](https://github.com/FrenkelS/Doom8088)** engine code.

1. `xtoviewangleTable` table.
2. `viewangletoxTable` table.
3. `VIEWANGLETOXMAX` value.
4. `screenheightarray` table.
5. `negonearray` table. 

## Compilation

```bash
gcc TableGenerator.c -o TableGenerator
```

## Usage & Examples

```sh
TableGenerator [VIEWWINDOWWIDTH]
TableGenerator 60
```

## Authors & Thanks

* **[FrenkelS](https://github.com/FrenkelS)**
