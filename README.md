# Spindash

##What is it?

Spindash is a 2D physics library for platform games that attempts to mimic the 
physics of the original Sonic the Hedgehog games. It's written with C linkage for
portability.

## Building and testing

* Check out the latest code from Git
* Create a "build" directory in the root and change to it
* Run: cmake .. && make
* Then run ./playground/spindash_playground

You will need OpenGL and SDL libraries installed

Playground is the sample test application for the library. If you find bugs, please
report them here: https://github.com/Kazade/Spindash/issues

If you fix bugs / add features, please submit a pull request on GitHub! You're awesome if you do!

## Concepts

### The World

The first step to using the library is to create the physics world. The world
defines the collidable geometry and contains characters and objects that collide with it
and each other. The world is scaled in comparison to the Sonic the Hedgehog games, it is
1/40th of the size (due to using GL units, rather than pixels).

### Characters

A character is an object that can move, jump and collide. It is represented by a "RayBox", a rectangular
shape defined by 6 rays, 2 up, 2 down, 1 forward and 1 back. Characters are different to other objects
in that they perform complex logic to determine their motion. 

### Springs

Springs are rectangular objects that will accelerate an object in a certain direction with a constant
force if the object collides with their "top" edge. They are represented by a Box. When creating a 
spring you must specify an angle, and a power.

### Triangle

A triangle is not an object, but is a CollisionPrimitive (like Boxes, Circles and RayBoxes), it cannot be 
moved, and is defined by 3 points. Triangles are the building blocks of the world geometry. If you are defining
landscape in your game, you should be using triangles.

