# Building

It is suggested that you build this project in a `build/` directory.

You can build this project with the following commands.

```sh
$ mkdir -p ./build/
>
```

```sh
$ cmake -DCMAKE_BUILD_TYPE=Release -B ./build/
>
```

```sh
$ cmake --build ./build/
>
```

You can then install the assets by running the cmake install command.

```sh
$ cmake --install ./build/
>
```
