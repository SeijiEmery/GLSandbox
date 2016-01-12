//
//  gamepad_text_input.cpp
//  GLSandbox
//
//  Created by semery on 1/10/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#include "gamepad_text_input.hpp"
#include "glm/glm.hpp"
#include <boost/math/constants/constants.hpp>

using namespace gl_sandbox;
using namespace modules;

GamepadTextInputModule::GamepadTextInputModule () :
m_onGamepadButtonPressed(input->onGamepadButtonPressed.connect([this](auto button){

})),
m_onGamepadAxesUpdate(input->onGamepadAxesUpdate.connect([this](auto axes) {
    
    auto left  = glm::vec2(axes[input::AXIS_LX], axes[input::AXIS_LY]);
    auto right = glm::vec2(axes[input::AXIS_RX], axes[input::AXIS_RY]);
    
    auto ltheta = glm::atan(left.y, left.x);
    auto rtheta = glm::atan(right.y, right.x);
    auto lr     = glm::length(left);
    auto rr     = glm::length(right);
    
    auto constexpr LOGGING = false;
    
    auto getSector = [=](float x, float y, float threshold = 0.5) {
        
        auto constexpr pi = boost::math::constants::pi<double>();
        auto constexpr segment_angle_inv = 6.0 / (2 * boost::math::constants::pi<double>());
        
        float r = sqrtf(x * x + y * y);
        float angle = fmodf(glm::atan(y, x) + pi, pi * 2);
        
        if (LOGGING) std::cout << "angle " << angle << ' ';
        
        return r < threshold ? 0 :
            (int)(floor(((double)angle) * segment_angle_inv)) + 1;
    };
    
    auto lsector = getSector(left.x, left.y);
    auto rsector = getSector(right.x, right.y);
    
    if (LOGGING) std::cout << "Left sector: " << lsector << ", Right sector: " << rsector << '\n';
    
    
    static constexpr char table [36] {
        'a','b','c','d','e','f',
        'g','h','i','j','k','l',
        'm','n','o','p','q','r',
        's','t','u','v','w','y',
        'x','z','0','1','2','3',
        '4','5','6','7','8','9'
    };
    
    auto lprev = m_lastSectors[0], rprev = m_lastSectors[1];
    
    
    auto n = -1;
    if (lprev != 0 && rprev != 0)
        n = (rprev - 1) * 6 + lprev - 1;
    else if (lsector != 0 && rsector != 0)
        n = (rsector - 1) * 6 + lsector - 1;
    
    
//    std::cout << "lprev " << lprev << " rprev " << rprev << '\n';
    if (n != -1 && (lprev != lsector || rprev != rsector)) {
        if (lsector == 0 || rsector == 0) {
            std::cout << "gamepad chr (ACCEPT): " << lprev << ", " << rprev << "(" << n << "): " << table[n] << "\n";
        } else {
            std::cout << "gamepad chr (switch): " << lprev << ", " << rprev << "(" << n << "): " << table[n] << "\n";
        }
    }
    
    
    
    
    
    m_lastSectors[0] = lsector;
    m_lastSectors[1] = rsector;
    
//
//    if (lr > 0.5) {
//        std::cout << "Left { angle: " << ltheta << ", r: " << lr << " } ";
//    }
//    if (rr > 0.5) {
//        std::cout << "Right { angle: " << rtheta << ", r: " << rr << " } ";
//    }
//    if (lr > 0.5 || rr > 0.5) {
//        std::cout << '\n';
//    }
    
}))
{}

void GamepadTextInputModule::drawFrame() {
    
}

void GamepadTextInputModule::onGamepadUpdate() {
    
}

















