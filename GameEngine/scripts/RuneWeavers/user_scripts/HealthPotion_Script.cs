using System;

namespace Eno
{
    public class HealthPotion_Script : ScriptBase
    {
        public float lifetime   = 8f;    // Seconds before the potion vanishes
        public float healAmount = 5f;   // HP restored on pickup

        private KinematicBody2D? _self;
        private Actor?           _player;
        private float            _timer          = 0f;
        private bool             _playerSearched = false;

        private const string HEALTH_RECOVER_PREFAB = "HealthRecover";

        public void Begin()
        {
            _self = Self<KinematicBody2D>();
            if (_self == null) return;

            _self.setAnimationLooping(false);
            uint frames = _self.getAnimationFrameCount();
            if (frames > 0)
                _self.setAnimationFrameDuration(lifetime / frames);
        }

        public void Update()
        {
            if (_self == null) return;
            if (PlayerController.IsGamePaused) return;

            // Despawn after lifetime
            _timer += Time.deltaTime;
            if (_timer >= lifetime)
            {
                FreeNode(_self);
                return;
            }

            // Lazy-find player once
            if (!_playerSearched)
            {
                _playerSearched = true;
                try { _player = GetNode<Actor>("Player"); }
                catch { _player = null; }
            }

            if (_player == null) return;

            // Pickup on contact
            if (_self.isColliding(_player))
            {
                _player.heal(healAmount);
                SpawnPrefab(HEALTH_RECOVER_PREFAB, _player.position.x, _player.position.y);
                FreeNode(_self);
            }
        }

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
    }
}
