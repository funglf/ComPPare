# Advanced Usage Demo

## Introduction
These advanced usage demos are for users who require more control and features over ComPPare. These examples will hopefully help you to understand via actual code so you can apply them to your benchmarks.

## [1. Custom Error Policy](1-custom_error_policy/README.md)
How to write your own Error Policy for your own data structures. As currently only Numerical Values, std Containers, and std::string are supported for native support. 

## [2. Google Benchmark](2-google_benchmark/README.md)
How to add google benchmark on top of ComPPare. 

## [3. Manual Timing](3-manual_timing/README.md)
How to ignore operations within the hotloop. For instance each loop requires a new setup.

## [4. DoNotOptimize](4-DoNotOptimize/README.md)
Deep Dive into the working principles of DoNotOptimize function and how you can use it for your benchmarks.

## [5. nvbench](5-nvbench/README.md)
How to add nvbench on top of ComPPare.