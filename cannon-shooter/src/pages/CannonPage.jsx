import { useEffect, useMemo, useRef, useState } from 'react'
import Scene from '../game/Scene'
import HUD from '../game/ui/HUD'
import useAimControls from '../game/utils/useAimControls'
import { TARGET_POSITIONS } from '../game/utils/constants'
import './CannonPage.css'

export default function CannonPage() {
  const aim = useAimControls()
  const sceneRef = useRef(null)
  const [score, setScore] = useState(0)
  const [shots, setShots] = useState(0)
  const [resetToken, setResetToken] = useState(0)
  const [targetsDown, setTargetsDown] = useState(0)
  const [status, setStatus] = useState('Line up a shot and clear all targets.')
  const totalTargets = useMemo(() => TARGET_POSITIONS.length, [])
  const accuracy = useMemo(
    () => (shots === 0 ? '--' : `${Math.round((targetsDown / shots) * 100)}%`),
    [shots, targetsDown],
  )

  const handleFire = () => {
    sceneRef.current?.fire()
    setShots((s) => s + 1)
    setStatus('Projectile launched! Track your arc in the HUD.')
  }

  const handleReset = () => {
    setScore(0)
    setShots(0)
    setTargetsDown(0)
    setResetToken((t) => t + 1)
    setStatus('Arena reset. Take aim and go for a clean run!')
  }

  const handleTargetDown = () => {
    setScore((s) => s + 100)
    setTargetsDown((t) => {
      const next = Math.min(totalTargets, t + 1)
      if (next === totalTargets) {
        setStatus('All targets down! Press reset to replay or chase accuracy.')
      } else {
        setStatus(`Target down! ${next}/${totalTargets} cleared.`)
      }
      return next
    })
  }

  useEffect(() => {
    const listener = (event) => {
      if (event.code === 'Space') {
        event.preventDefault()
        handleFire()
      }
      if (event.code === 'KeyR') {
        event.preventDefault()
        handleReset()
      }
    }
    window.addEventListener('keydown', listener)
    return () => window.removeEventListener('keydown', listener)
  }, [])

  useEffect(() => {
    window.parent?.postMessage({ type: 'READY' }, '*')
  }, [])

  return (
    <div className="cannon-page">
      <Scene
        ref={sceneRef}
        yaw={aim.yaw}
        pitch={aim.pitch}
        onTargetDown={handleTargetDown}
        resetToken={resetToken}
      />
      <HUD
        yaw={aim.yaw}
        pitch={aim.pitch}
        score={score}
        shots={shots}
        targets={`${targetsDown}/${totalTargets}`}
        accuracy={accuracy}
        status={status}
        onReset={handleReset}
        onFire={handleFire}
        aimHandlers={aim}
      />
    </div>
  )
}
