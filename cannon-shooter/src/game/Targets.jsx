import { useEffect, useMemo, useRef, useState } from 'react'
import { RigidBody } from '@react-three/rapier'
import { TARGET_FALL_LIMIT, TARGET_POSITIONS } from './utils/constants'

function TargetBlock({ position, bodyRef }) {
  return (
    <RigidBody
      ref={bodyRef}
      position={position}
      colliders="cuboid"
      restitution={0.1}
      friction={1}
      angularDamping={0.4}
      linearDamping={0.2}
      canSleep={true}
    >
      <mesh castShadow receiveShadow>
        <boxGeometry args={[0.7, 1, 0.7]} />
        <meshStandardMaterial color="#fbbf24" roughness={0.45} />
      </mesh>
    </RigidBody>
  )
}

export default function Targets({ onTargetDown, resetToken }) {
  const bodyRefs = useMemo(
    () => TARGET_POSITIONS.map(() => ({ ref: useRef(), fallen: false })),
    [],
  )
  const [resetCount, setResetCount] = useState(0)

  useEffect(() => {
    setResetCount((c) => c + 1)
    bodyRefs.forEach((entry, index) => {
      const body = entry.ref.current
      entry.fallen = false
      if (!body) return
      const [x, y, z] = TARGET_POSITIONS[index]
      body.setTranslation({ x, y, z }, true)
      body.setLinvel({ x: 0, y: 0, z: 0 }, true)
      body.setAngvel({ x: 0, y: 0, z: 0 }, true)
      body.wakeUp()
    })
  }, [resetToken, bodyRefs])

  useEffect(() => {
    const id = setInterval(() => {
      bodyRefs.forEach((entry, index) => {
        const body = entry.ref.current
        if (!body || entry.fallen) return
        const translation = body.translation()
        if (translation.y < TARGET_FALL_LIMIT) {
          entry.fallen = true
          onTargetDown?.(index)
        }
      })
    }, 200)

    return () => clearInterval(id)
  }, [bodyRefs, onTargetDown])

  return (
    <group key={resetCount}>
      {bodyRefs.map((entry, index) => (
        <TargetBlock key={index} position={TARGET_POSITIONS[index]} bodyRef={entry.ref} />
      ))}
    </group>
  )
}
