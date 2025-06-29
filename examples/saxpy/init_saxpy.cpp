#include <string_view>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <random>

#include "init_saxpy.hpp"

SaxpyConfig init_saxpy(int argc, char **argv)
{
    SaxpyConfig cfg;

    // Parse flags
    for (int i = 1; i < argc; ++i)
    {
        std::string_view key = argv[i];

        if (key == "--size" && i + 1 < argc)
        {
            cfg.N = 1ULL << std::stoull(argv[++i]);
        }
        else if (key == "--scalar" && i + 1 < argc)
        {
            cfg.a = std::stof(argv[++i]);
        }
        else if (key == "--iter" && i + 1 < argc)
        {
            cfg.iters = std::stoull(argv[++i]);
        }
        else if (key == "--tol" && i + 1 < argc)
        {
            cfg.tol = std::stod(argv[++i]);
        }
        else if (key == "--warmup")
        {
            cfg.warmup = true;
        }
        else if (key == "--no-warmup")
        {
            cfg.warmup = false;
        }
        else
        {
            throw std::invalid_argument("Unknown argument: " + std::string(key));
        }
    }

    // Randomize scalar if not provided
    if (std::isnan(cfg.a))
    {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-5.0f, 5.0f);
        cfg.a = dist(rng);
    }

    // Randomly initialize x and y
    cfg.x.resize(cfg.N);
    cfg.y.resize(cfg.N);
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1e6f, 1e6f);
    for (size_t i = 0; i < cfg.N; ++i)
    {
        cfg.x[i] = dist(rng);
        cfg.y[i] = dist(rng);
    }

    // Print config
    std::cout << "\n=== SAXPY Benchmark Parameters ===\n";
    std::cout << "Vector size (N)     : " << cfg.N << '\n';
    std::cout << "Scalar a            : " << cfg.a << '\n';
    std::cout << "Iterations          : " << cfg.iters << '\n';
    std::cout << "Error tolerance     : " << cfg.tol << '\n';
    std::cout << "===================================\n\n";

    return cfg;
}
