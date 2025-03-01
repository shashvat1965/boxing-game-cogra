# Boxing Game : OpenGL | GLUT | Assimp

Welcome to the Boxing Game project for the course IS F311: Computer Graphics (BITS Pilani)! This project is a 3D boxing game developed using the Cogra framework. The game features realistic boxing mechanics, player animations, and an immersive environment.

## Features

- Boxing mechanics
- Player animations
- Immersive 2D environment

## Dependencies

The following dependencies are required to run the project:

- GLUT
- Assimp
- libpng
- OpenGL

## Installation

To install and run the project, follow these steps:

1. Clone the repository:
   ```sh
   git clone https://github.com/yourusername/boxing-game-cogra.git
   ```
2. Navigate to the project directory:
   ```sh
   cd boxing-game-cogra
   ```
3. Install the required dependencies on Ubuntu:
   ```sh
   sudo apt-get update
   sudo apt-get install freeglut3-dev libassimp-dev libpng-dev
   ```
4. Build and run the program using

   ```sh
   make # compile the program
   ./boxing-game-cogra # to run the program

   make run # directly compile and run
   ```

## How to use

- Use the WASD keys to move the camera.
- Select a player by clicking <kbd>1</kbd> or <kbd>2</kbd>
- Punch the other player by clicking <kbd>P</kbd> on your keyboard

## Research on Model Rendering

During the development of this project, we conducted extensive research on how to render model objects using the Assimp library. Initially, we attempted to use FBX files, which already contain animations. However, this approach proved to be out of the scope of this project and was overly complicated, leading to several hours of wasted effort.

Ultimately, we decided to simplify our approach and focus on rendering static models without pre-defined animations. This allowed us to streamline the development process and achieve our project goals more efficiently.
