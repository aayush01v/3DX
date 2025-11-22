import { forwardRef, useImperativeHandle, useMemo, useRef } from 'react'
import { Quaternion, Vector3 } from 'three'
import { useFrame } from '@react-three/fiber'

const muzzleOffset = new Vector3(0, 0, 1.15)

const Cannon = forwardRef(function Cannon({ yaw, pitch }, ref) {
  const baseRef = useRef()
  const barrelRef = useRef()
  const quaternion = useMemo(() => new Quaternion(), [])
  const worldDir = useMemo(() => new Vector3(), [])
  const muzzleWorld = useMemo(() => new Vector3(), [])

  useImperativeHandle(ref, () => ({
    getMuzzleTransform() {
      if (!barrelRef.current) return null
      barrelRef.current.getWorldQuaternion(quaternion)
      muzzleWorld.copy(muzzleOffset)
      barrelRef.current.localToWorld(muzzleWorld)
      worldDir.set(0, 0, 1).applyQuaternion(quaternion).normalize()
      return { position: muzzleWorld.clone(), direction: worldDir.clone() }
    },
  }))

  useFrame(() => {
    if (baseRef.current) {
      baseRef.current.rotation.y = yaw
    }
    if (barrelRef.current) {
      barrelRef.current.rotation.x = -pitch
    }
  })

  return (
    <group position={[0, 0.35, 0]}>
      <group ref={baseRef}>
        <mesh castShadow receiveShadow>
          <cylinderGeometry args={[0.6, 0.7, 0.6, 16]} />
          <meshStandardMaterial color="#4dc1ff" metalness={0.35} roughness={0.5} />
        </mesh>
        <group position={[0, 0.35, 0]}>
          <mesh castShadow receiveShadow position={[0, 0.05, 0]}>
            <cylinderGeometry args={[0.15, 0.3, 0.2, 12]} />
            <meshStandardMaterial color="#0ea5e9" metalness={0.2} roughness={0.4} />
          </mesh>
          <group>
            <mesh
              ref={barrelRef}
              castShadow
              receiveShadow
              position={[0, 0.1, 0]}
            >
              <boxGeometry args={[0.35, 0.35, 2.3]} />
              <meshStandardMaterial color="#8b5cf6" roughness={0.35} metalness={0.25} />
            </mesh>
          </group>
        </group>
      </group>
    </group>
  )
})

export default Cannon
