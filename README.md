# lobot_servo

A C library to interface HiWonder's
[LX-15D](https://www.hiwonder.hk/collections/servo/products/hiwonder-lx-15d-serial-bus-servo)
Serial Bus Servo

Additionally, a simple utility program that uses this library to communicate the
servo is provided in the
[utils](https://github.com/xqinx/lobot_servo/tree/main/utils) folder

---
## Build
```bash
cmake -S . -B build
cmake --build build
```

---
## Quick start
Assuming a servo is connected to you host machine on port `/dev/ttyUSB0`, the
following code snippet demonstrates how to use this library
```c
int main(void)
{
    port = lobot_port_open("/dev/ttyUSB0");
    if(!port) exit(-1);

    /* check the ID of the connected servo, using broadcast address 0xFE */
    uint8 id = 0;
    lobot_get_id(port, 0xFE, &id);

    /* read current servo position */
    uint16_t pos = 0;
    lobot_get_pos(port, id, &pos);

    /* set servo to a new position, which is current position plus 50
     * the value 50 here is raw control value to the servo's register,
     * which, for LX-15D, can be translated to degrees according to its
     * datasheet as:
     *
     *   50 * 240 / 1000
     *
     * last argument specify the time needed for this movement. A value of 0
     * instruct the servo to move at its maximum speed
     */
    lobot_set_pos(port, id, pos + 50, 0);

    lobot_port_close(port);
}
```

---
## ROS2
We also provide a ros2 package under branch `ros2_foxy`. As the name suggests it
supports `foxy` distro. Other ROS2 distros are not tested but should generally work

To use this package, add `lobot_servo` as a dependency in your ROS2
project's `package.xml`, as well as your `CMakeLists.txt`
