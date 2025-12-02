package com.carx.simulator;

/**
 * Native library wrapper for CarX Physics Engine
 * 
 * This class provides Java bindings to the C++ physics engine.
 * The native code is in jni/main.cpp
 */
public class NativeLib {
    
    // Load native library
    static {
        System.loadLibrary("carx-native");
    }
    
    // Singleton instance
    private static NativeLib instance;
    
    public static NativeLib getInstance() {
        if (instance == null) {
            instance = new NativeLib();
        }
        return instance;
    }
    
    private NativeLib() {}
    
    // ========================================================================
    // Native Methods
    // ========================================================================
    
    /** Initialize the physics engine */
    public native void init();
    
    /** Update physics simulation */
    public native void update(float deltaTime);
    
    /** Set throttle input (0 to 1) */
    public native void setThrottle(float value);
    
    /** Set steering input (-1 to 1) */
    public native void setSteering(float value);
    
    /** Set brake input (0 to 1) */
    public native void setBrake(float value);
    
    /** Shift gear up */
    public native void shiftUp();
    
    /** Shift gear down */
    public native void shiftDown();
    
    /** Set terrain type (0=flat, 1=hills, 2=mountains, 3=track) */
    public native void setTerrain(int type);
    
    /** Reset car to starting position */
    public native void reset();
    
    /**
     * Get car state array:
     * [0] = posX, [1] = posY, [2] = posZ,
     * [3] = rotation, [4] = speed, [5] = rpm,
     * [6] = gear, [7] = onGround, [8] = throttle, [9] = steering
     */
    public native float[] getCarState();
    
    /** Get terrain height at position */
    public native float getTerrainHeight(float x, float z);
    
    /** Cleanup native resources */
    public native void cleanup();
    
    // ========================================================================
    // Helper Methods
    // ========================================================================
    
    public static class CarState {
        public float posX, posY, posZ;
        public float rotation;
        public float speed;
        public float rpm;
        public int gear;
        public boolean onGround;
        public float throttle;
        public float steering;
        
        public static CarState fromArray(float[] arr) {
            CarState state = new CarState();
            if (arr != null && arr.length >= 10) {
                state.posX = arr[0];
                state.posY = arr[1];
                state.posZ = arr[2];
                state.rotation = arr[3];
                state.speed = arr[4];
                state.rpm = arr[5];
                state.gear = (int) arr[6];
                state.onGround = arr[7] > 0.5f;
                state.throttle = arr[8];
                state.steering = arr[9];
            }
            return state;
        }
        
        public String getGearName() {
            if (gear == 0) return "N";
            if (gear == -1) return "R1";
            if (gear == -2) return "R2";
            return String.valueOf(gear);
        }
        
        public float getSpeedKmh() {
            return speed * 3.6f;
        }
        
        public float getSpeedMph() {
            return speed * 2.237f;
        }
    }
    
    public CarState getState() {
        return CarState.fromArray(getCarState());
    }
}
