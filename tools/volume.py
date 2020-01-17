import math
import random

def sampled(estimator_fn):
    def inner(*args, **kwargs):
        sample_counts = [1, 10, 100, 1000]

        estimates = []
        for sample_count in sample_counts:
            estimate = 0
            for i in range(sample_count):
                estimate += estimator_fn(*args, **kwargs) / sample_count
            estimates.append(estimate)

        return estimates
    return inner

def sample(low, high):
    return random.random() * (high - low) + low

def sample_tau(low, high, sigma_t):
    s = -math.log(random.random() / sigma_t)
    return s + low

def uniform_emitter_fn(radiance):
    return lambda x: radiance

def distance(x, y):
    return y - x

# t(x, x') = 1
# sigma_s = sigma_a = 0
def emission_only(emission, x, y):
    x_prime = sample(x, y)
    return emission(x_prime) * distance(x, y)

def constant_sigma_a_tau_fn(sigma_a):
    return lambda x, y: math.exp(-sigma_a * distance(x, y))

def constant_Le_fn(radiance):
    return lambda x, w: radiance

# emission = 0
# sigma_s = 0
# tau(x, y) = exp(-sigma_a * distance(x, y))
def homogenous_absorption(tau, Le, x, y):
    w = y - x
    return tau(x, y) * Le(y, w)

def full_transmittance_tau_fn():
    return lambda x, y: 1.

@sampled
def emission_plus_homogenous_absorption(tau, emission, Le, x, y):
    x_prime = sample(x, y)
    return distance(x, y) * tau(x, x_prime) * emission(x_prime)

@sampled
def emission_plus_homogenous_absorption__sample_tau(sigma_t, emission, Le, x, y):
    x_prime = sample_tau(x, y, sigma_t)
    if x_prime < y:
        return emission(x_prime) / sigma_t
    return 0


def run():
    estimate = emission_only(
        uniform_emitter_fn(1.),
        0,
        5
    )
    print(estimate)

    estimate = emission_plus_homogenous_absorption(
        full_transmittance_tau_fn(),
        uniform_emitter_fn(1.),
        constant_Le_fn(1.),
        0,
        5
    )
    print(estimate)

    estimate = homogenous_absorption(
        constant_sigma_a_tau_fn(0.1),
        constant_Le_fn(1.),
        0,
        5
    )
    print(estimate)

    estimate = emission_plus_homogenous_absorption(
        constant_sigma_a_tau_fn(0.1),
        uniform_emitter_fn(1.),
        constant_Le_fn(1.),
        0,
        5
    )
    print(estimate)

    estimate = emission_plus_homogenous_absorption__sample_tau(
        0.1,
        uniform_emitter_fn(1.),
        constant_Le_fn(1.),
        0,
        5
    )
    print(estimate)


if __name__ == "__main__":
    run()
