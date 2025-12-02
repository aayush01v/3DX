/**
 * CarX Simulator - Universal Scene Loader (JavaScript/Three.js)
 * ==============================================================
 * Cross-platform scene definition loader for Three.js applications.
 * 
 * This module reads scene_definition.json and provides a unified API
 * for creating scenes that match Blender, Unreal, Unity, etc.
 * 
 * Usage:
 * ------
 *   import { SceneLoader, loadSceneDefinition } from './scene_loader.js';
 *   
 *   const scene = await loadSceneDefinition('../scene_definition.json');
 *   const carData = scene.getCar();
 *   const terrainData = scene.getTerrain();
 * 
 * Author: CarX Simulator Team
 * License: MIT
 * Version: 1.0.0
 */

// ============================================================================
// DATA CLASSES
// ============================================================================

class Vector3 {
    constructor(x = 0, y = 0, z = 0) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    
    static fromArray(arr) {
        return new Vector3(arr[0] || 0, arr[1] || 0, arr[2] || 0);
    }
    
    toArray() {
        return [this.x, this.y, this.z];
    }
    
    toThreeVector() {
        return new THREE.Vector3(this.x, this.y, this.z);
    }
}

class Transform {
    constructor(position, rotation, scale) {
        this.position = position || new Vector3();
        this.rotation = rotation || new Vector3();
        this.scale = scale || new Vector3(1, 1, 1);
    }
    
    static fromObject(obj) {
        return new Transform(
            Vector3.fromArray(obj.position || [0, 0, 0]),
            Vector3.fromArray(obj.rotation || [0, 0, 0]),
            Vector3.fromArray(obj.scale || [1, 1, 1])
        );
    }
}

class Material {
    constructor(color = '#FFFFFF', metalness = 0, roughness = 0.5, emission = null) {
        this.color = color;
        this.metalness = metalness;
        this.roughness = roughness;
        this.emission = emission;
    }
    
    static fromObject(obj) {
        return new Material(
            obj.color || '#FFFFFF',
            obj.metalness || 0,
            obj.roughness || 0.5,
            obj.emission || null
        );
    }
    
    toThreeMaterial() {
        const params = {
            color: this.color,
            metalness: this.metalness,
            roughness: this.roughness
        };
        
        if (this.emission) {
            params.emissive = this.emission;
            params.emissiveIntensity = 0.5;
        }
        
        return new THREE.MeshStandardMaterial(params);
    }
}

class Wheel {
    constructor(name, position, steering = false, drive = false) {
        this.name = name;
        this.position = position;
        this.steering = steering;
        this.drive = drive;
    }
    
    static fromObject(obj) {
        return new Wheel(
            obj.name || 'wheel',
            Vector3.fromArray(obj.position || [0, 0, 0]),
            obj.steering || false,
            obj.drive || false
        );
    }
}

class BodyPart {
    constructor(name, type, dimensions, position, rotation, material) {
        this.name = name;
        this.type = type;
        this.dimensions = dimensions;
        this.position = position;
        this.rotation = rotation || new Vector3();
        this.material = material || new Material();
    }
    
    static fromObject(obj) {
        return new BodyPart(
            obj.name || 'part',
            obj.type || 'box',
            Vector3.fromArray(obj.dimensions || [1, 1, 1]),
            Vector3.fromArray(obj.position || [0, 0, 0]),
            Vector3.fromArray(obj.rotation || [0, 0, 0]),
            Material.fromObject(obj.material || {})
        );
    }
    
    createThreeMesh() {
        let geometry;
        
        switch (this.type) {
            case 'box':
                geometry = new THREE.BoxGeometry(
                    this.dimensions.x,
                    this.dimensions.y,
                    this.dimensions.z
                );
                break;
            case 'cylinder':
                geometry = new THREE.CylinderGeometry(
                    this.dimensions.x / 2,
                    this.dimensions.x / 2,
                    this.dimensions.y,
                    16
                );
                break;
            default:
                geometry = new THREE.BoxGeometry(1, 1, 1);
        }
        
        const material = this.material.toThreeMaterial();
        const mesh = new THREE.Mesh(geometry, material);
        
        mesh.position.set(this.position.x, this.position.y, this.position.z);
        mesh.rotation.set(this.rotation.x, this.rotation.y, this.rotation.z);
        mesh.name = this.name;
        mesh.castShadow = true;
        mesh.receiveShadow = true;
        
        return mesh;
    }
}

class CarPhysics {
    constructor(mass, wheelbase, trackWidth, wheelRadius, groundClearance, centerOfMass) {
        this.mass = mass || 1200;
        this.wheelbase = wheelbase || 2.6;
        this.trackWidth = trackWidth || 1.8;
        this.wheelRadius = wheelRadius || 0.35;
        this.groundClearance = groundClearance || 0.5;
        this.centerOfMass = centerOfMass || new Vector3(0, 0.4, 0);
    }
    
    static fromObject(obj) {
        return new CarPhysics(
            obj.mass,
            obj.wheelbase,
            obj.track_width,
            obj.wheel_radius,
            obj.ground_clearance,
            Vector3.fromArray(obj.center_of_mass || [0, 0.4, 0])
        );
    }
}

class CarDefinition {
    constructor(name, transform, physics, wheels, bodyParts, headlights, taillights, modelPath) {
        this.name = name;
        this.transform = transform;
        this.physics = physics;
        this.wheels = wheels;
        this.bodyParts = bodyParts;
        this.headlights = headlights;
        this.taillights = taillights;
        this.modelPath = modelPath;
    }
    
    static fromObject(obj, assets = {}) {
        const lights = obj.lights || {};
        const assetInfo = assets.car || {};
        
        return new CarDefinition(
            obj.name || 'Car',
            Transform.fromObject(obj.transform || {}),
            CarPhysics.fromObject(obj.physics || {}),
            (obj.wheels || []).map(w => Wheel.fromObject(w)),
            (obj.body_parts || []).map(p => BodyPart.fromObject(p)),
            (lights.headlights || []).map(l => ({ position: Vector3.fromArray(l.position || [0, 0, 0]), color: l.color || '#FFFFAA' })),
            (lights.taillights || []).map(l => ({ position: Vector3.fromArray(l.position || [0, 0, 0]), color: l.color || '#FF0000' })),
            assetInfo.model || null
        );
    }
}

class TerrainNoise {
    constructor(type, octaves, frequency, amplitude) {
        this.type = type || 'perlin';
        this.octaves = octaves || 3;
        this.frequency = frequency || [0.02, 0.015, 0.01];
        this.amplitude = amplitude || [2, 1.5, 3];
    }
    
    static fromObject(obj) {
        return new TerrainNoise(
            obj.type,
            obj.octaves,
            obj.frequency,
            obj.amplitude
        );
    }
}

class TerrainDefinition {
    constructor(type, size, segments, heightScale, noise, material) {
        this.type = type || 'procedural';
        this.size = size || 400;
        this.segments = segments || 120;
        this.heightScale = heightScale || 8;
        this.noise = noise || new TerrainNoise();
        this.material = material || new Material('#4A7D32', 0, 0.85);
    }
    
    static fromObject(obj) {
        return new TerrainDefinition(
            obj.type,
            obj.size,
            obj.segments,
            obj.height_scale,
            TerrainNoise.fromObject(obj.noise || {}),
            Material.fromObject(obj.material || {})
        );
    }
    
    /**
     * Calculate terrain height at position using noise parameters.
     * This MUST match the Blender implementation exactly!
     */
    getHeight(x, z) {
        let h = 0;
        const freq = this.noise.frequency;
        const amp = this.noise.amplitude;
        
        for (let i = 0; i < freq.length; i++) {
            h += Math.sin(x * freq[i]) * amp[i];
            h += Math.cos(z * freq[i] * 0.75) * amp[i] * 0.75;
        }
        
        return h;
    }
}

class EnvironmentDefinition {
    constructor(skyTopColor, skyBottomColor, sun, ambientColor, ambientIntensity, fog) {
        this.skyTopColor = skyTopColor || '#87CEEB';
        this.skyBottomColor = skyBottomColor || '#E0F7FA';
        this.sun = sun || { position: new Vector3(50, 100, 50), color: '#FFF5E6', intensity: 1.5 };
        this.ambientColor = ambientColor || '#FFFFFF';
        this.ambientIntensity = ambientIntensity || 0.4;
        this.fog = fog || { enabled: true, color: '#CCCCCC', near: 100, far: 500 };
    }
    
    static fromObject(obj) {
        const sky = obj.sky || {};
        const sun = obj.sun || {};
        const ambient = obj.ambient || {};
        const fog = obj.fog || {};
        
        return new EnvironmentDefinition(
            sky.top_color,
            sky.bottom_color,
            {
                position: Vector3.fromArray(sun.position || [50, 100, 50]),
                rotation: Vector3.fromArray(sun.rotation || [45, 30, 0]),
                color: sun.color || '#FFF5E6',
                intensity: sun.intensity || 1.5,
                shadows: sun.shadows !== false
            },
            ambient.color,
            ambient.intensity,
            {
                enabled: fog.enabled !== false,
                color: fog.color || '#CCCCCC',
                near: fog.near || 100,
                far: fog.far || 500
            }
        );
    }
}

class CameraDefinition {
    constructor(position, target, fov, near, far, follow) {
        this.position = position || new Vector3(15, 10, 15);
        this.target = target || new Vector3(0, 2, 0);
        this.fov = fov || 60;
        this.near = near || 0.1;
        this.far = far || 2000;
        this.follow = follow || { distance: 25, height: 10, angle: 0, pitch: 0.4, smooth: 0.1 };
    }
    
    static fromObject(obj) {
        const def = obj.default || {};
        const follow = obj.follow || {};
        
        return new CameraDefinition(
            Vector3.fromArray(def.position || [15, 10, 15]),
            Vector3.fromArray(def.target || [0, 2, 0]),
            def.fov,
            def.near,
            def.far,
            {
                distance: follow.distance || 25,
                height: follow.height || 10,
                angle: follow.angle || 0,
                pitch: follow.pitch || 0.4,
                smooth: follow.smooth || 0.1
            }
        );
    }
}

class VegetationDefinition {
    constructor(enabled, treeCount, spawnMin, spawnMax, exclusion, scaleRange, modelPath) {
        this.enabled = enabled !== false;
        this.treeCount = treeCount || 50;
        this.spawnMin = spawnMin || [-80, -80];
        this.spawnMax = spawnMax || [80, 80];
        this.exclusion = exclusion || { center: [0, 0], radiusX: 10, radiusZ: 30 };
        this.scaleRange = scaleRange || [0.8, 1.2];
        this.modelPath = modelPath;
    }
    
    static fromObject(obj, assets = {}) {
        const trees = obj.trees || {};
        const spawn = trees.spawn_area || {};
        const exclusion = trees.exclusion_zone || {};
        const assetInfo = assets.tree || {};
        
        return new VegetationDefinition(
            obj.enabled,
            trees.count,
            spawn.min,
            spawn.max,
            {
                center: exclusion.center || [0, 0],
                radiusX: exclusion.radius_x || 10,
                radiusZ: exclusion.radius_z || 30
            },
            trees.scale_range,
            assetInfo.model
        );
    }
}

// ============================================================================
// SCENE DEFINITION
// ============================================================================

class SceneDefinition {
    constructor(data, basePath = '') {
        this.rawData = data;
        this.basePath = basePath;
        this.version = data.version || '1.0.0';
        this.metadata = data.metadata || {};
        
        // Coordinate system
        const coord = data.coordinate_system || {};
        this.upAxis = coord.up || 'Y';
        this.forwardAxis = coord.forward || '-Z';
        this.units = coord.units || 'meters';
        
        // Assets
        this.assets = data.assets || {};
        
        // Parse components
        this.car = CarDefinition.fromObject(data.car || {}, this.assets);
        this.terrain = TerrainDefinition.fromObject(data.terrain || {});
        this.environment = EnvironmentDefinition.fromObject(data.environment || {});
        this.camera = CameraDefinition.fromObject(data.camera || {});
        this.vegetation = VegetationDefinition.fromObject(data.vegetation || {}, this.assets);
    }
    
    getAssetPath(assetKey) {
        const asset = this.assets[assetKey] || {};
        const modelPath = asset.model;
        if (modelPath && this.basePath) {
            return this.basePath + '/' + modelPath;
        }
        return modelPath;
    }
    
    getCar() { return this.car; }
    getTerrain() { return this.terrain; }
    getEnvironment() { return this.environment; }
    getCamera() { return this.camera; }
    getVegetation() { return this.vegetation; }
    
    toJSON() { return this.rawData; }
}

// ============================================================================
// LOADER FUNCTIONS
// ============================================================================

/**
 * Load scene definition from JSON file.
 * @param {string} url - URL to scene_definition.json
 * @returns {Promise<SceneDefinition>}
 */
async function loadSceneDefinition(url = '../scene_definition.json') {
    try {
        const response = await fetch(url);
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }
        const data = await response.json();
        
        // Extract base path from URL
        const basePath = url.substring(0, url.lastIndexOf('/'));
        
        console.log(`✓ Loaded scene definition v${data.version || '1.0.0'}`);
        return new SceneDefinition(data, basePath);
    } catch (error) {
        console.warn(`⚠ Could not load scene definition: ${error.message}`);
        console.log('Using fallback scene definition');
        return new SceneDefinition(getFallbackSceneDefinition());
    }
}

/**
 * Get fallback scene definition if JSON fails to load.
 */
function getFallbackSceneDefinition() {
    return {
        version: '1.0.0',
        metadata: { name: 'CarX Simulator (Fallback)' },
        coordinate_system: { up: 'Y', forward: '-Z', units: 'meters' },
        assets: {
            car: { model: 'lowpoly_game_model.glb', fallback: 'procedural' },
            tree: { model: 'tree.glb', fallback: 'procedural' }
        },
        car: {
            name: 'Car',
            transform: { position: [0, 0.85, 0], rotation: [0, 0, 0], scale: [1, 1, 1] },
            physics: {
                mass: 1200,
                wheelbase: 2.6,
                track_width: 1.8,
                wheel_radius: 0.35,
                ground_clearance: 0.5
            },
            wheels: [
                { name: 'wheel_FL', position: [-0.9, 0.35, 1.3], steering: true },
                { name: 'wheel_FR', position: [0.9, 0.35, 1.3], steering: true },
                { name: 'wheel_RL', position: [-0.9, 0.35, -1.3], drive: true },
                { name: 'wheel_RR', position: [0.9, 0.35, -1.3], drive: true }
            ],
            body_parts: [
                { name: 'body', type: 'box', dimensions: [2.0, 0.8, 4.0], position: [0, 0.6, 0], material: { color: '#FF6B35', metalness: 0.7, roughness: 0.3 } },
                { name: 'cabin', type: 'box', dimensions: [1.6, 0.7, 2.0], position: [0, 1.15, -0.3], material: { color: '#2C3E50', metalness: 0.8, roughness: 0.2 } }
            ],
            lights: {
                headlights: [{ position: [-0.7, 0.7, 2.05] }, { position: [0.7, 0.7, 2.05] }],
                taillights: [{ position: [-0.7, 0.7, -2.05] }, { position: [0.7, 0.7, -2.05] }]
            }
        },
        terrain: {
            type: 'procedural',
            size: 400,
            segments: 120,
            height_scale: 8,
            noise: {
                type: 'perlin',
                frequency: [0.02, 0.015, 0.01],
                amplitude: [2, 1.5, 3]
            },
            material: { color: '#4A7D32', roughness: 0.85 }
        },
        environment: {
            sky: { top_color: '#87CEEB', bottom_color: '#E0F7FA' },
            sun: { position: [50, 100, 50], rotation: [45, 30, 0], color: '#FFF5E6', intensity: 1.5, shadows: true },
            ambient: { color: '#FFFFFF', intensity: 0.4 },
            fog: { enabled: true, color: '#CCCCCC', near: 100, far: 500 }
        },
        camera: {
            default: { position: [15, 10, 15], target: [0, 2, 0], fov: 60 },
            follow: { distance: 25, height: 10, pitch: 0.4 }
        },
        vegetation: {
            enabled: true,
            trees: { count: 50, spawn_area: { min: [-80, -80], max: [80, 80] } }
        }
    };
}

// ============================================================================
// THREE.JS SCENE BUILDER
// ============================================================================

/**
 * Build Three.js scene from scene definition.
 */
class ThreeJSSceneBuilder {
    constructor(sceneDef) {
        this.sceneDef = sceneDef;
        this.scene = null;
        this.car = null;
        this.terrain = null;
    }
    
    /**
     * Create procedural car from scene definition.
     */
    createProceduralCar() {
        const carDef = this.sceneDef.getCar();
        const carGroup = new THREE.Group();
        carGroup.name = carDef.name;
        
        // Create body parts
        for (const part of carDef.bodyParts) {
            const mesh = part.createThreeMesh();
            carGroup.add(mesh);
        }
        
        // Create wheels
        const wheelGeometry = new THREE.CylinderGeometry(
            carDef.physics.wheelRadius,
            carDef.physics.wheelRadius,
            0.3,
            16
        );
        const wheelMaterial = new THREE.MeshStandardMaterial({
            color: 0x111111,
            metalness: 0.5,
            roughness: 0.8
        });
        
        for (const wheelDef of carDef.wheels) {
            const wheel = new THREE.Mesh(wheelGeometry, wheelMaterial);
            wheel.rotation.z = Math.PI / 2;
            wheel.position.set(
                wheelDef.position.x,
                wheelDef.position.y,
                wheelDef.position.z
            );
            wheel.name = wheelDef.name;
            wheel.castShadow = true;
            carGroup.add(wheel);
        }
        
        // Create lights
        for (const light of carDef.headlights) {
            const lightMesh = new THREE.Mesh(
                new THREE.BoxGeometry(0.3, 0.2, 0.1),
                new THREE.MeshStandardMaterial({
                    color: light.color,
                    emissive: light.color,
                    emissiveIntensity: 0.5
                })
            );
            lightMesh.position.set(light.position.x, light.position.y, light.position.z);
            carGroup.add(lightMesh);
        }
        
        for (const light of carDef.taillights) {
            const lightMesh = new THREE.Mesh(
                new THREE.BoxGeometry(0.3, 0.2, 0.1),
                new THREE.MeshStandardMaterial({
                    color: light.color,
                    emissive: light.color,
                    emissiveIntensity: 0.5
                })
            );
            lightMesh.position.set(light.position.x, light.position.y, light.position.z);
            carGroup.add(lightMesh);
        }
        
        // Apply transform
        const transform = carDef.transform;
        carGroup.position.set(transform.position.x, transform.position.y, transform.position.z);
        carGroup.rotation.set(transform.rotation.x, transform.rotation.y, transform.rotation.z);
        carGroup.scale.set(transform.scale.x, transform.scale.y, transform.scale.z);
        
        this.car = carGroup;
        return carGroup;
    }
    
    /**
     * Create terrain from scene definition.
     */
    createTerrain() {
        const terrainDef = this.sceneDef.getTerrain();
        
        const geometry = new THREE.PlaneGeometry(
            terrainDef.size,
            terrainDef.size,
            terrainDef.segments,
            terrainDef.segments
        );
        
        // Apply height function
        const positions = geometry.attributes.position.array;
        for (let i = 0; i < positions.length; i += 3) {
            const x = positions[i];
            const z = positions[i + 1]; // PlaneGeometry uses Y for "forward"
            positions[i + 2] = terrainDef.getHeight(x, z);
        }
        
        geometry.computeVertexNormals();
        
        const material = terrainDef.material.toThreeMaterial();
        const terrain = new THREE.Mesh(geometry, material);
        terrain.rotation.x = -Math.PI / 2;
        terrain.receiveShadow = true;
        terrain.name = 'Terrain';
        
        this.terrain = terrain;
        return terrain;
    }
    
    /**
     * Create environment (lights, sky, fog).
     */
    createEnvironment(scene) {
        const envDef = this.sceneDef.getEnvironment();
        
        // Sun light
        const sunLight = new THREE.DirectionalLight(envDef.sun.color, envDef.sun.intensity);
        sunLight.position.set(
            envDef.sun.position.x,
            envDef.sun.position.y,
            envDef.sun.position.z
        );
        sunLight.castShadow = envDef.sun.shadows;
        sunLight.shadow.mapSize.width = 2048;
        sunLight.shadow.mapSize.height = 2048;
        sunLight.name = 'Sun';
        scene.add(sunLight);
        
        // Ambient light
        const ambientLight = new THREE.AmbientLight(envDef.ambientColor, envDef.ambientIntensity);
        scene.add(ambientLight);
        
        // Fog
        if (envDef.fog.enabled) {
            scene.fog = new THREE.Fog(envDef.fog.color, envDef.fog.near, envDef.fog.far);
        }
        
        // Sky color
        scene.background = new THREE.Color(envDef.skyTopColor);
        
        return { sunLight, ambientLight };
    }
    
    /**
     * Create camera from scene definition.
     */
    createCamera() {
        const camDef = this.sceneDef.getCamera();
        
        const camera = new THREE.PerspectiveCamera(
            camDef.fov,
            window.innerWidth / window.innerHeight,
            camDef.near,
            camDef.far
        );
        
        camera.position.set(
            camDef.position.x,
            camDef.position.y,
            camDef.position.z
        );
        
        camera.lookAt(
            camDef.target.x,
            camDef.target.y,
            camDef.target.z
        );
        
        return camera;
    }
}

// ============================================================================
// EXPORTS
// ============================================================================

// For ES6 modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        SceneDefinition,
        loadSceneDefinition,
        getFallbackSceneDefinition,
        ThreeJSSceneBuilder,
        Vector3,
        Transform,
        Material,
        CarDefinition,
        TerrainDefinition,
        EnvironmentDefinition,
        CameraDefinition,
        VegetationDefinition
    };
}

// For browser global
if (typeof window !== 'undefined') {
    window.CarXSceneLoader = {
        SceneDefinition,
        loadSceneDefinition,
        getFallbackSceneDefinition,
        ThreeJSSceneBuilder,
        Vector3,
        Transform,
        Material,
        CarDefinition,
        TerrainDefinition,
        EnvironmentDefinition,
        CameraDefinition,
        VegetationDefinition
    };
}
