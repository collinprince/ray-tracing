#ifndef THREADING_H
#define THREADING_H

#include <algorithm>
#include <thread>

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "rtweekend.h"

color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;

    // if we've exceeded the ray bounce limit, no more light is gathered
    if (depth <= 0) {
        return color(0, 0, 0);
    }
    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * ray_color(scattered, world, depth - 1);
        }
        // Q: we return black if scatter returns false?
        return color(0, 0, 0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

struct thread_params {
    int line_index;
    int thread_index;
    int samples_per_pixel;
    int image_width;
    int image_height;
    int max_depth;
    int N;
    hittable_list* world;
    camera* cam;
    std::vector<std::vector<std::string>>* thread_outputs;
};

struct scene_params {
    int samples_per_pixel;
    int image_width;
    int image_height;
    int max_depth;
    hittable_list* world;
    camera* cam;
};

// thread function for naive approach where we give each thread
// image_height / N lines to complete
void thread_work(thread_params tp) {
    auto thread_index = tp.thread_index;
    auto samples_per_pixel = tp.samples_per_pixel;
    auto image_width = tp.image_width;
    auto image_height = tp.image_height;
    auto max_depth = tp.max_depth;
    auto N = tp.N;
    auto world = tp.world;
    auto cam = tp.cam;
    auto thread_outputs = tp.thread_outputs;

    std::vector<std::string> colors;

    int thread_chunk_size = ceil((double)image_height / N);

    int j_bottom = thread_chunk_size * thread_index;
    int j_top = std::min(thread_chunk_size * (thread_index + 1), image_height);

    for (int j = j_top - 1; j >= j_bottom; --j) {
        std::cerr << "\rScanlines remaining for thread " << thread_index << ": "
                  << j - j_bottom << "\n";
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam->get_ray(u, v);
                pixel_color += ray_color(r, *world, max_depth);
            }
            colors.push_back(color_to_string(pixel_color, samples_per_pixel));
        }
    }
    (*thread_outputs)[thread_index] = colors;
}

// thread work function that only gives each thread one line to complete
void one_line_thread(thread_params tp) {
    auto line_index = tp.line_index;
    auto thread_index = tp.thread_index;
    auto samples_per_pixel = tp.samples_per_pixel;
    auto image_width = tp.image_width;
    auto image_height = tp.image_height;
    auto max_depth = tp.max_depth;
    auto N = tp.N;
    auto world = tp.world;
    auto cam = tp.cam;
    auto thread_outputs = tp.thread_outputs;

    auto j = line_index;

    for (int i = 0; i < image_width; ++i) {
        color pixel_color(0, 0, 0);
        for (int s = 0; s < samples_per_pixel; ++s) {
            auto u = (i + random_double()) / (image_width - 1);
            auto v = (j + random_double()) / (image_height - 1);
            ray r = cam->get_ray(u, v);
            pixel_color += ray_color(r, *world, max_depth);
        }
        (*thread_outputs)[thread_index][i] =
            color_to_string(pixel_color, samples_per_pixel);
    }
};

// function to handle creating a batch of N threads and printing
// their output in correct order to std::cout
void multi_threaded(int N, scene_params& sp) {
    std::vector<std::thread> threads(N);
    std::vector<std::vector<std::string>> thread_outputs(
        N, std::vector<std::string>(sp.image_width, ""));

    std::cerr << "Starting multi-threaded render with " << N << " threads.\n";
    for (int j = sp.image_height - 1; j >= 0; j -= N) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        // account for last cycle of work where
        // there may be less than N lines to complete
        int num_threads = std::min(N, j + 1);
        for (int i = 0; i < num_threads; ++i) {
            thread_params tp;
            tp.thread_index = i;
            tp.line_index = j - i;
            tp.samples_per_pixel = sp.samples_per_pixel;
            tp.image_height = sp.image_height;
            tp.image_width = sp.image_width;
            tp.max_depth = sp.max_depth;
            tp.N = N;
            tp.world = sp.world;
            tp.cam = sp.cam;
            tp.thread_outputs = &thread_outputs;
            std::thread tmp(one_line_thread, std::move(tp));
            threads[i] = std::move(tmp);
        }

        // rejoin all threads
        for (int i = 0; i < num_threads; ++i) {
            threads[i].join();
        }

        // print thread's work in this batch
        for (int i = 0; i < num_threads; ++i) {
            for (auto s : thread_outputs[i]) {
                std::cout << s;
            }
        }
    }
}

// handle rendering in single-threaded manner
void single_threaded(scene_params& sp) {
    auto image_width = sp.image_width;
    auto image_height = sp.image_height;
    auto samples_per_pixel = sp.samples_per_pixel;
    auto max_depth = sp.max_depth;
    auto world = *(sp.world);
    auto cam = *(sp.cam);

    std::cerr << "Starting single-threaded render.\n";
    for (int j = sp.image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < sp.image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }
}

#endif