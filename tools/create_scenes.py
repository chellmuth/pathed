from pathlib import Path
from xml.dom.minidom import parse

import click

def _remove_integrator(root):
    integrators = root.getElementsByTagName("integrator")
    assert len(integrators) == 1
    root.removeChild(integrators[0])

def _find_sensor(root):
    sensors = root.getElementsByTagName("sensor")

    assert len(sensors) == 1
    sensor, = sensors
    return sensor

def _find_film(sensor):
    films = sensor.getElementsByTagName("film")

    assert len(films) == 1
    film, = films
    return film

def _replace_sampler(dom, root):
    sensor = _find_sensor(root)
    samplers = sensor.getElementsByTagName("sampler")

    assert len(samplers) == 1
    original_sampler, = samplers

    sensor.removeChild(original_sampler)

    sampler_element = dom.createElement("sampler")
    sampler_element.setAttribute("type", "independent")

    _add_basic_element(dom, sampler_element, "integer", "sampleCount", "$spp")

    sensor.appendChild(sampler_element)

def _replace_film(dom, root):
    sensor = _find_sensor(root)
    original_film = _find_film(sensor)

    sensor.removeChild(original_film)

    film_element = dom.createElement("film")
    film_element.setAttribute("type", "hdrfilm")
    _add_basic_element(dom, film_element, "integer", "width", "$width")
    _add_basic_element(dom, film_element, "integer", "height", "$height")
    _add_basic_element(dom, film_element, "boolean", "banner", "false")

    filter_element = dom.createElement("rfilter")
    filter_element.setAttribute("type", "box")
    film_element.appendChild(filter_element)

    sensor.appendChild(film_element)

def _add_basic_element(dom, root, element_type, name, value, prepend=False):
    element = dom.createElement(element_type)
    element.setAttribute("name", name)
    element.setAttribute("value", value)

    if prepend:
        root.insertBefore(element, root.firstChild)
    else:
        root.appendChild(element)

def _add_defaults(dom, root, defaults):
    for name, value in defaults.items():
        _add_basic_element(dom, root, "default", name, value, prepend=True)

def _find_default_size(root):
    sensor = _find_sensor(root)
    film = _find_film(sensor)

    width = None
    height = None
    for element in film.getElementsByTagName("integer"):
        if element.hasAttribute("name"):
            if element.getAttribute("name") == "width":
                width = element.getAttribute("value")
            if element.getAttribute("name") == "height":
                height = element.getAttribute("value")

    assert width
    assert height

    return width, height

def _common_changes(dom, scene_root):
    width, height = _find_default_size(scene_root)
    _add_defaults(dom, scene_root, {
        "spp": "1",
        "width": width,
        "height": height
    })

    _remove_integrator(scene_root)
    _replace_sampler(dom, scene_root)
    _replace_film(dom, scene_root)


def create_path(dom, scene_root):
    _common_changes(dom, scene_root)

    integrator_element = dom.createElement("integrator")
    integrator_element.setAttribute("type", "path")

    _add_basic_element(dom, integrator_element, "integer", "maxDepth", "3")
    _add_basic_element(dom, integrator_element, "boolean", "hideEmitters", "true")
    _add_basic_element(dom, integrator_element, "boolean", "strictNormals", "true")

    scene_root.appendChild(integrator_element)

def create_training(dom, scene_root):
    _common_changes(dom, scene_root)
    _add_defaults(dom, scene_root, {
        "x": "-1",
        "y": "-1",
        "seed": "noseed",
        "minutesAllotment": 0
    })

    path_element = dom.createElement("integrator")
    path_element.setAttribute("type", "path")

    _add_basic_element(dom, path_element, "integer", "maxDepth", "2")
    _add_basic_element(dom, path_element, "boolean", "hideEmitters", "false")
    _add_basic_element(dom, path_element, "boolean", "strictNormals", "true")

    fisheye_element = dom.createElement("integrator")
    fisheye_element.setAttribute("type", "fisheye")
    _add_basic_element(dom, fisheye_element, "integer", "maxDepth", "2")
    _add_basic_element(dom, fisheye_element, "boolean", "strictNormals", "true")
    _add_basic_element(dom, fisheye_element, "boolean", "hideEmitters", "false")

    _add_basic_element(dom, fisheye_element, "integer", "x", "$x")
    _add_basic_element(dom, fisheye_element, "integer", "y", "$y")
    _add_basic_element(dom, fisheye_element, "integer", "minutesAllotment", "$minutesAllotment")
    _add_basic_element(dom, fisheye_element, "string", "seed", "$seed")

    fisheye_element.appendChild(path_element)
    scene_root.appendChild(fisheye_element)

def create_neural(dom, scene_root):
    _common_changes(dom, scene_root)
    _add_defaults(dom, scene_root, {
        "x": "-1",
        "y": "-1",
        "integrator": "neural_fast",
    })

    integrator_element = dom.createElement("integrator")
    integrator_element.setAttribute("type", "$integrator")

    _add_basic_element(dom, integrator_element, "integer", "maxDepth", "3")
    _add_basic_element(dom, integrator_element, "boolean", "hideEmitters", "true")
    _add_basic_element(dom, integrator_element, "boolean", "strictNormals", "true")

    _add_basic_element(dom, integrator_element, "integer", "x", "$x")
    _add_basic_element(dom, integrator_element, "integer", "y", "$y")

    scene_root.appendChild(integrator_element)

def process_scene(path):
    scenarios = [
        (create_path, "scene-path.xml"),
        (create_training, "scene-training.xml"),
        (create_neural, "scene-neural.xml"),
    ]

    for scenario_fn, output_name in scenarios:
        dom = parse(open(path))
        root = dom.documentElement

        scenario_fn(dom, root)

        with open(path.parent / output_name, "w") as f:
            dom.writexml(f)

@click.command()
@click.argument("diffuse_scene", type=Path)
def run(diffuse_scene):
    process_scene(diffuse_scene)

if __name__ == "__main__":
    # read_scene(Path("/home/cjh/workpad/Dropbox/research/scenes/staircase2/scene.xml"))
    run()
