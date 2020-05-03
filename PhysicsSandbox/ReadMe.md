Project - PhysicsSandbox

## Checklist 

- [x] Be able to attach user data to a rigidbody and/or collider to hook the physics system up to game code.

- [ ] Add collision/contact events
    - [ ] `OnOverlapBegin`
    - [ ] `OnOverlapStay`
    - [ ] `OnOverlapLeave`

- [ ] Add Trigger Volumes
    - [ ] `OnTriggerEnter`
    - [ ] `OnTriggerStay`
    - [ ] `OnTriggerLeave`

- [ ] Support Physics Layers
    - [ ] Be able to specify which layer a rigid body and/or collider belongs to.
    - [ ] Be able to enable or disable collision between two given layers.
    - [ ] Only process collisions if the two objects are allowed to interact
    - [ ] Only process triggers if the two objects are on the same layer

- [ ] **Optional**: Support axis locks.
    - [ ] Support `X` and `Y` axis locks, only allowing movement in those directions.
    - [ ] Support `Rotation` lock preventing the object from rotating. 


Controls
1 - Create disc
2 - Draw polygon
Others in tooltip and on screen
