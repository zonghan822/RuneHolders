/*============================================================================================
* @file         SpellEffect.cs
* @brief        Drives the lifecycle of a spawned spell prefab.
*
*               Attach this script to RedSpell, YellowSpell, and BlueSpell prefabs.
*               On spawn the node enters the "windup" state. After WindupDuration seconds
*               it transitions to "explosion". After ExplosionDuration seconds the node
*               destroys itself.
*
* Required states on the prefab's StateMachineComponent:
*   windup    – charging animation before the spell goes off
*   explosion – the actual spell burst; deal damage here via other systems
*============================================================================================*/

namespace Eno
{
    public class SpellEffect : ScriptBase
    {
        // ── Tuneable values ──────────────────────────────────────────────────────
        private const float WINDUP_DURATION    = 0.5f;
        private const float EXPLOSION_DURATION = 0.8f;

        // ── Runtime state ────────────────────────────────────────────────────────
        private float  _timer = 0f;
        private string _phase = "windup";

        // ── Script callbacks ─────────────────────────────────────────────────────
        public void Begin()
        {
            _timer = 0f;
            _phase = "windup";
            Self<Sprite2D>()?.changeState("windup");
        }

        public void Update()
        {
            _timer += Time.deltaTime;

            if (_phase == "windup" && _timer >= WINDUP_DURATION)
            {
                _phase = "explosion";
                _timer = 0f;
                Self<Sprite2D>()?.changeState("explosion");
            }
            else if (_phase == "explosion" && _timer >= EXPLOSION_DURATION)
            {
                FreeNode(Self<Sprite2D>());
            }
        }
    }
}
