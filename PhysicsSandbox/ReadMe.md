Project - PhysicsSandbox

------

FIX CURSOR BLINK

## Checklist
- [ ] Angular Update - Add members to `Rigidbody2D`
    - [x] Add `float m_rotationInRadians`
    - [x] Add `float m_angularVelocity`   
    - [x] Add `float m_frameTorque`       
    - [x] Add `float m_moment`
    - [ ] Add appropriate getter/setter methods
- [-] `Rigidbody2D::CalculateMoment` to calculate moment based on attached collider and mass
    - [x] `virtual float Collider2D::CalculateMoment( float mass )`
    - [-] Implement `DiscCollider::CalculateMoment`
    - [ ] Implement `PolygonCollider::CalculateMoment`
    - [-] When setting **mass** or **collider** on a rigidbody - recalculate your moment.
        - *Note: When setting mass, you can do less work by taking advantage of the ratio of new mass to old mass should match the ratios of the moments*. 
- [ ] Rotating a `Rigidbody2D` properly updates the **world shape** of the collider.
    - [x] Add controls to rotate a selected object
        - Uses **Q** and **E** 
    - [x] Add controls to update the rotational velocity of an object
        - Uses **R**, **T**, and **Y** to increase, decrease, and reset
- [x] `Rigidbody2D` now do angular calculations during their update.
    - [x] Use torque to compute an angular acceleration
    - [x] Apply angular acceleration to update angular velocity
    - [x] Apply angular velocity to update rotation
- [ ] Update `Rigidbody2D::GetImpactVelocity` to take into account rotational velocity.
- [-] Update `ApplyImpulseAt` to apply impulse to torque based on positions. 
- [-] Update impulse calculations to take into account rotational forces, see PDF in lnks
- [ ] Update tooltip to show...
    - [ ] Moment of Inertia
    - [x] Current Rotation (degrees)
    - [x] Current Angular Velocity


------

Controls
All are same as recommended except drag, I used `:` and `'`