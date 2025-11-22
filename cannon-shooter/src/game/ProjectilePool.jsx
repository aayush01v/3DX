import { createRef, forwardRef, useImperativeHandle, useMemo, useRef, useState } from 'react'
import { Color } from 'three'
import { RigidBody } from '@react-three/rapier'
import { PROJECTILE_POOL_SIZE, PROJECTILE_SPEED } from './utils/constants'

const ProjectilePool = forwardRef(function ProjectilePool({ getMuzzleTransform }, ref) {
  const bodies = useRef(Array.from({ length: PROJECTILE_POOL_SIZE }, () => createRef()))
  const [activeIndex, setActiveIndex] = useState(0)
  const projectileColor = useMemo(() => new Color('#f97316'), [])

  useImperativeHandle(ref, () => ({
    fire() {
      if (!getMuzzleTransform) return
      const muzzle = getMuzzleTransform()
      if (!muzzle) return

      const bodyRef = bodies.current[activeIndex]?.current
      if (!bodyRef) return

      bodyRef.setTranslation(
        { x: muzzle.position.x, y: muzzle.position.y, z: muzzle.position.z },
        true,
      )
      bodyRef.setLinvel(
        {
          x: muzzle.direction.x * PROJECTILE_SPEED,
          y: muzzle.direction.y * PROJECTILE_SPEED,
          z: muzzle.direction.z * PROJECTILE_SPEED,
        },
        true,
      )
      bodyRef.setAngvel({ x: 0, y: 0, z: 0 }, true)
      bodyRef.wakeUp()
      setActiveIndex((i) => (i + 1) % PROJECTILE_POOL_SIZE)
    },
  }))

  return (
    <group>
      {bodies.current.map((body, index) => (
        <RigidBody
          key={index}
          ref={body}
          gravityScale={1}
          restitution={0.35}
          friction={0.9}
          colliders="ball"
          position={[0, 0.5, 0]}
          enabledRotations={[true, true, true]}
        >
          <mesh castShadow receiveShadow>
            <sphereGeometry args={[0.15, 16, 16]} />
            <meshStandardMaterial color={projectileColor} emissive={projectileColor} emissiveIntensity={0.15} />
          </mesh>
        </RigidBody>
      ))}
    </group>
  )
})

export default ProjectilePool
