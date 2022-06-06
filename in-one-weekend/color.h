#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include <string>

#include "vec3.h"

void write_color(std::ostream& out, color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // divide the color by the number of samples and gamma-correct for gamma=2.0
    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    // write the translated [0,255] value of each color component
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

std::string color_to_string(color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // divide the color by the number of samples and gamma-correct for gamma=2.0
    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    std::string r_str =
        std::to_string(static_cast<int>(256 * clamp(r, 0.0, 0.999)));
    std::string g_str =
        std::to_string(static_cast<int>(256 * clamp(g, 0.0, 0.999)));
    std::string b_str =
        std::to_string(static_cast<int>(256 * clamp(b, 0.0, 0.999)));

    return r_str + ' ' + g_str + ' ' + b_str + '\n';
}

#endif