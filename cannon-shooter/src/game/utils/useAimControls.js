import { useCallback, useRef, useState } from 'react'
import { clamp } from 'three/src/math/MathUtils.js'
import { MAX_PITCH, MIN_PITCH } from './constants'

export default function useAimControls(initialYaw = 0, initialPitch = (25 * Math.PI) / 180) {
  const [yaw, setYaw] = useState(initialYaw)
  const [pitch, setPitch] = useState(initialPitch)
  const dragging = useRef(false)
  const lastPoint = useRef({ x: 0, y: 0 })

  const updateAim = useCallback((dx, dy) => {
    setYaw((y) => y + dx * 0.01)
    setPitch((p) => clamp(p + dy * 0.01, MIN_PITCH, MAX_PITCH))
  }, [])

  const onPointerDown = useCallback((event) => {
    dragging.current = true
    lastPoint.current = { x: event.clientX, y: event.clientY }
  }, [])

  const onPointerMove = useCallback(
    (event) => {
      if (!dragging.current) return
      const dx = event.clientX - lastPoint.current.x
      const dy = event.clientY - lastPoint.current.y
      lastPoint.current = { x: event.clientX, y: event.clientY }
      updateAim(dx, dy)
    },
    [updateAim],
  )

  const endDrag = useCallback(() => {
    dragging.current = false
  }, [])

  const handleWheel = useCallback((event) => {
    event.preventDefault()
    const delta = event.deltaY > 0 ? 0.05 : -0.05
    setPitch((p) => clamp(p + delta, MIN_PITCH, MAX_PITCH))
  }, [])

  return {
    yaw,
    pitch,
    setYaw,
    setPitch,
    onPointerDown,
    onPointerMove,
    onPointerUp: endDrag,
    onPointerLeave: endDrag,
    onWheel: handleWheel,
  }
}
