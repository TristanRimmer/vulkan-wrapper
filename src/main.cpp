#include <iostream>

/*
 * Rendering Engine Concept
 *
 * Features:
 *
 * - Elegant Asset Wrapper:
 *   - Inputs:
 *    -> .obj file
 *    -> .png file
 *   - Role:
 *    -> Creates its own Vertex/Index Buffer
 *    -> Chooses a Sampling method
 *    -> Has option for Local Transformations
 *      - For EG, a Wheel needs a local Transformation of it spinning
 *
 * - Asset Loader:
 *   - Should hopefully boil down to something as simple as:
 *    -> Vec<Assets> assets; update(assets); load(assets);
 *
 * - Inputs Wrapper
 *   - Exposing the inner workings to the raw GLFW inputs would be pretty
 *     disgusting
 *   - Event System?
 *
 * - Movement Controller:
 *   - As name suggets
 *   - Possibly different types of movement controller that detail how each
 *     WASD/Space/Shift/Mouse input should be handled -> Interface
 *   - Perhaps the movement controller gets a reference to a special type of
 *     asset for the 'Player'
 *
 * - Need an elegant way to isolate the graphics 'back end' and the logical
 *   'front end'. How might this be done?
 */

int main() { std::cout << "Hello Vulkan! \n"; }
