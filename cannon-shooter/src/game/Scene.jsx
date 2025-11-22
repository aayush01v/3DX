import { forwardRef, Suspense, useImperativeHandle, useMemo, useRef } from 'react'
import { Canvas } from '@react-three/fiber'
import { Environment, OrbitControls } from '@react-three/drei'
import { Physics, RigidBody } from '@react-three/rapier'
import Cannon from './Cannon'
import ProjectilePool from './ProjectilePool'
import Targets from './Targets'
import { GROUND_SIZE } from './utils/constants'

const Scene = forwardRef(function Scene(
  { yaw, pitch, onTargetDown, resetToken, enableDebugCamera = false },
  ref,
) {
  const cannonRef = useRef(null)
  const projectileRef = useRef(null)

  const getMuzzleTransform = useMemo(
    () => () => cannonRef.current?.getMuzzleTransform?.(),
    [],
  )

  useImperativeHandle(ref, () => ({
    fire() {
      projectileRef.current?.fire()
    },
  }))

  return (
    <Canvas shadows camera={{ position: [8, 6, 12], fov: 50 }}>
      <color attach="background" args={[0.02, 0.04, 0.07]} />
      <ambientLight intensity={0.35} />
      <directionalLight
        castShadow
        position={[10, 12, 6]}
        intensity={1.1}
        shadow-mapSize-width={1024}
        shadow-mapSize-height={1024}
      />
      <Physics gravity={[0, -9.81, 0]}>
        <group position={[0, 0, 0]}>
          <Cannon ref={cannonRef} yaw={yaw} pitch={pitch} />
          <ProjectilePool ref={projectileRef} getMuzzleTransform={getMuzzleTransform} />
          <Targets onTargetDown={onTargetDown} resetToken={resetToken} />
        </group>
        <RigidBody type="fixed" restitution={0.05} friction={1} position={[0, 0, 0]}>
          <mesh receiveShadow rotation-x={-Math.PI / 2}>
            <planeGeometry args={[GROUND_SIZE, GROUND_SIZE]} />
            <meshStandardMaterial color="#0b1120" />
          </mesh>
        </RigidBody>
      </Physics>
      <Suspense fallback={null}>
        <Environment preset="sunset" />
      </Suspense>
      {enableDebugCamera && <OrbitControls />}
    </Canvas>
  )
})

export default Scene
