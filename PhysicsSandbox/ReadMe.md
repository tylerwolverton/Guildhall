Project - PhysicsSandbox

## Checklist 

- [x] Be able to attach user data to a rigidbody and/or collider to hook the physics system up to game code.

- [x] Add collision/contact events
    - [x] `OnOverlapBegin`
    - [x] `OnOverlapStay`
    - [x] `OnOverlapLeave`

- [x] Add Trigger Volumes
    - [x] `OnTriggerEnter`
    - [x] `OnTriggerStay`
    - [x] `OnTriggerLeave`

- [x] Support Physics Layers
    - [x] Be able to specify which layer a rigid body and/or collider belongs to.
    - [x] Be able to enable or disable collision between two given layers.
    - [x] Only process collisions if the two objects are allowed to interact
    - [x] Only process triggers if the two objects are on the same layer

- [ ] **Optional**: Support axis locks.
    - [ ] Support `X` and `Y` axis locks, only allowing movement in those directions.
    - [ ] Support `Rotation` lock preventing the object from rotating. 


Controls
1 - Create disc
2 - Draw polygon
3 - Create disc trigger
Others in tooltip and on screen
