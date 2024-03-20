# Build Your Own DSL using LLVM

## Getting started

### Prerequisites
- LLVM 16
- CMake
- Ninja

or

- Docker
- (optional) VS Code with Dev Containers Extension (ms-vscode-remote.remote-containers)

### Docker

We provide a docker image with an LLVM installation.
While you can build and run your own LLVM stack locally, your mileage may vary.
The docker image is a stable environment to experiment with the examples in this repository.

To use the provided image, first make sure that docker is installed and running on your system.
Run `run_docker.sh` to fetch and run the pre-built docker container.
You are now inside the running docker container and can run the scripts inside the different exercise folders.

To build the docker image yourself (e.g. you want to add packages) run `build_docker.sh` in the `docker/` folder.

### Use with VS Code

#### Recommended/required extensions
- C/C++ Extension Pack (ms-vscode.cpptools-extension-pack)
- clangd (llvm-vs-code-extensions.vscode-clangd)
- Dev Containers (ms-vscode-remote.remote-containers)

#### Using With Docker
Start the Docker container using the provided `run_docker.sh` script.

Open container in VS Code: `CTRL+SHIFT+P`, type and run `Dev Containers: Attach to Running Container`, select the running container.
Open the repository in `/home/<username>/llvm_dsl` (e.g. by using `CTRL+K+O`).
Check that the extensions that are recommended above are also installed in the docker container (`CTRL+SHIFT+X`).

Now you should be able to use the CMake Tools to build the project.
Do so by either opening only the relevant subdirectory or selecting the relevant CMakeLists.txt when asked.

#### Using Without Docker
Just open the currently relevant subdirectory from the repository in VS Code and use the CMake extension to build the project.

#### Troubleshooting

When using VS Code with rootless docker (on a systemd based system...) find your docker socket location with: `systemctl status docker.socket --user`.
Then, add this to your workspace settings (`.vscode/settings.json`):
```json
{
    "docker.environment": {
        "DOCKER_HOST": "unix:///run/user/1000/docker.sock"
    },
}
```
