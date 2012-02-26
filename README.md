# Spindash

##What is it?

Spindash is a 2D physics library for platform games that attempts to mimic the 
physics of the original Sonic the Hedgehog games. It's written with C linkage for
portability.

## Concepts

### The World

The first step to using the library is to create the physics world. The world
defines the collidable geometry and contains characters and objects that collide with it
and each other.

### Characters

A character is an object that can move, jump and collide.

### Objects

An object is uncontrollable and only has simple physical rules applied (e.g. collision, gravity)

