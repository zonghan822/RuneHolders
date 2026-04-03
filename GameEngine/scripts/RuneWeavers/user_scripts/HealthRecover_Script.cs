namespace Eno
{
    public class HealthRecover_Script : ScriptBase
    {
        public float animDuration = 1f;   // Should match the sprite animation length

        private const float RUMBLE_DURATION = 0.2f;

        private Sprite2D? _self;
        private Actor?    _player;
        private float     _timer          = 0f;
        private bool      _playerSearched = false;

        public void Begin()
        {
            _self = Self<Sprite2D>();
            if (_self == null) return;

            Input.setGamepadRumble(0.3f, 0.3f);
        }

        public void Update()
        {
            if (_self == null) return;
            
            _self.z_index = 4;
            // Lazy-find player once
            if (!_playerSearched)
            {
                _playerSearched = true;
                try { _player = GetNode<Actor>("Player"); }
                catch { _player = null; }
            }

            // Follow player position
            if (_player != null)
                _self.position = _player.position;

            _timer += Time.deltaTime;
            if (_timer >= RUMBLE_DURATION && _timer - Time.deltaTime < RUMBLE_DURATION)
                Input.setGamepadRumble(0f, 0f);
            if (_timer >= animDuration)
                FreeNode(_self);
        }

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
    }
}
