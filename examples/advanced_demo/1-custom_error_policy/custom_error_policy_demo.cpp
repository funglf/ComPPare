#include <string>

#include "data_structure.hpp"
#include "custom_policy.hpp"

#include <comppare/comppare.hpp>

void car1(car &c)
{
    comppare::DoNotOptimize(c);
    HOTLOOPSTART;
    c.make = "Toyota";
    c.model = "Prius";
    c.mileage = 6006.13f;
    c.year = 2020;
    HOTLOOPEND;
}

void car2(car &c)
{
    comppare::DoNotOptimize(c);
    HOTLOOPSTART;
    c.make = "Toyota";
    c.model = "Corolla";
    c.mileage = 101010.1f;
    c.year = 1998;
    HOTLOOPEND;
}

void car3(car &c)
{
    comppare::DoNotOptimize(c);
    HOTLOOPSTART;
    c.make = "Porsche";
    c.model = "911";
    c.mileage = 1963.0f;
    c.year = 2020;
    HOTLOOPEND;
}

int main(int argc, char **argv)
{
    auto compare = comppare::make_comppare<comppare::set_policy<car, IsSameBrand>>();

    compare.set_reference("Toyota Prius", car1);
    compare.add("Toyota Corolla", car2);
    compare.add("Porsche 911", car3);

    compare.run(argc, argv);
}