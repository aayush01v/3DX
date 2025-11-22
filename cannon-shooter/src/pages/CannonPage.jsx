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
  const totalTargets = useMemo(() => TARGET_POSITIONS.length, [])

  const handleFire = () => {
    sceneRef.current?.fire()
    setShots((s) => s + 1)
  }

  const handleReset = () => {
    setScore(0)
    setShots(0)
    setTargetsDown(0)
    setResetToken((t) => t + 1)
  }

  const handleTargetDown = () => {
    setScore((s) => s + 100)
    setTargetsDown((t) => Math.min(totalTargets, t + 1))
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
        onReset={handleReset}
        onFire={handleFire}
        aimHandlers={aim}
      />
    </div>
  )
}
