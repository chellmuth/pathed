import json
import numpy as np

def sample_mean(samples):
    return np.mean(samples)

def sample_variance(samples):
    mean = sample_mean(samples)
    return sum(
        (sample - mean) ** 2
        for sample in samples
    ) / (len(samples) - 1)

if __name__ == "__main__":
    samples = np.array(json.load(open("samples.json")))
    print(sample_variance(samples))
