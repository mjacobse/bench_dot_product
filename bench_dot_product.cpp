#include <benchmark/benchmark.h>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>


extern "C" {
    double ddot_(const int* n, const double* x, const int* incx,
                 const double* y, const int* incy);
}

struct dotProduct {
    double operator()(const std::vector<double>& v,
                      const std::vector<double>& w) const {
        return std::inner_product(v.cbegin(), v.cend(), w.cbegin(), 0.0);
    }

    double operator()(const std::vector<double>& v) const {
        const int length = v.size();
        double sum = 0.0;
        for (int i = 0; i < length; i += 2) {
            sum += v[i] * v[i + 1];
        }
        return sum;
    }
};

struct dotProductBLAS {
    double operator()(const std::vector<double>& v,
                      const std::vector<double>& w) const {
        const int n = v.size();
        const int incx = 1;
        const int incy = 1;
        return ddot_(&n, v.data(), &incx, w.data(), &incy);
    }

    double operator()(const std::vector<double>& v) const {
        const int n = v.size();
        const int incx = 2;
        const int incy = 2;
        return ddot_(&n, v.data(), &incx, v.data() + 1, &incy);
    }
};

template<class dotProduct>
void dotProductSeparate(benchmark::State& state) {
    const int length = state.range(0);
    const int rng_seed = state.range(1);
    const int num_bytes = 2 * length * sizeof(double);

    std::vector<double> v;
    std::vector<double> w;
    v.reserve(length);
    w.reserve(length);
    std::mt19937 rng(rng_seed);
    std::normal_distribution<double> distribution;
    for (int i = 0; i < length; ++i) {
        v.emplace_back(distribution(rng));
        w.emplace_back(distribution(rng));
    }

    for (auto _ : state) {
        const double dot_product = dotProduct()(v, w);
        benchmark::DoNotOptimize(dot_product);
    }
    state.SetBytesProcessed(state.iterations() * num_bytes);
}

template<class dotProduct>
void dotProductCombined(benchmark::State& state) {
    const int length = state.range(0);
    const int rng_seed = state.range(1);
    const int num_bytes = 2 * length * sizeof(double);

    std::vector<double> v;
    v.reserve(length * 2);
    std::mt19937 rng(rng_seed);
    std::normal_distribution<double> distribution;
    for (int i = 0; i < length; ++i) {
        v.emplace_back(distribution(rng));
    }

    for (auto _ : state) {
        const double dot_product = dotProduct()(v);
        benchmark::DoNotOptimize(dot_product);
    }
    state.SetBytesProcessed(state.iterations() * num_bytes);
}

void separate(benchmark::State& state) {
    dotProductSeparate<dotProduct>(state);
}

void separateBLAS(benchmark::State& state) {
    dotProductSeparate<dotProductBLAS>(state);
}

void combined(benchmark::State& state) {
    dotProductCombined<dotProduct>(state);
}

void combinedBLAS(benchmark::State& state) {
    dotProductCombined<dotProductBLAS>(state);
}

void CustomArguments(benchmark::internal::Benchmark* b) {
    const int rng_seed = 0;  // does not really matter
    for (int length_exponent = 6; length_exponent <= 22; length_exponent += 1) {
        b->Args({static_cast<int>(std::pow(2, length_exponent)), rng_seed});
    }
}
BENCHMARK(separate)->Apply(CustomArguments);
BENCHMARK(separateBLAS)->Apply(CustomArguments);
BENCHMARK(combined)->Apply(CustomArguments);
BENCHMARK(combinedBLAS)->Apply(CustomArguments);
BENCHMARK_MAIN();
