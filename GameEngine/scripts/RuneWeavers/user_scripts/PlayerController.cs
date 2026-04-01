using System;
using System.Collections.Generic;

namespace Eno
{
    public class PlayerController : ScriptBase
    {
        // Movement speed in world units per second
        public float speed = 300f;

        public float attackDuration = 0.5f;
        public float castDuration   = 0.5f;
        public float hurtDuration   = 0.4f;

        private Actor     _player = null!;
        private Canvas?   _canvas       = null;
        private Sprite2D? _hpBarFull;
        private bool      _hpBarReady   = false;
        private float     _hpBarInitW   = 0f;
        private float     _hpBarInitH   = 0f;
        private float     _hpBarLocalX  = 0f;
        private float     _hpBarLocalY  = 0f;
        private float     _attackTimer  = 0f;
        private float     _hurtTimer   = 0f;
        private bool      _isAttacking = false;
        private bool      _isHurt      = false;

        private Node2D?[] _scrollNodes    = Array.Empty<Node2D?>();
        private uint      _spellbookUIID    = 0;
        private bool      _spellbookOpen    = false;
        private uint      _winLoseCanvasID  = 0;
        private uint      _enemyInfoID        = 0;
        private uint      _enemyInfoPopupID   = 0;
        private uint      _enemyInfoPopupBgID = 0;
        private bool      _enemyInfoOpen      = false;

        // In-game pause flag
        public static bool IsGamePaused = false;

        // Canvas interface
        //private bool _UIPopUp = false;

        public void Begin()
        {
            _player = Self<Actor>()!;

            // ACTOR nodes have physics — keep gravity off for top-down movement
            _player.useGravity      = false;
            _player.isKinematic     = false;
            _player.collisionGroup  = 1;
            _player.maxHealth       = 5f;
            _player.health      = 5f;
            _player.damage      = 1f;

            
            _hpBarFull = GetNode<Sprite2D>("PlayerHealthBar_Full");
            try { _canvas = GetNode<Canvas>("Gameplay_UI"); } catch { }

            _scrollNodes = new Node2D?[Scroll_Dialouge_Script.ScrollNames.Length];
            for (int i = 0; i < _scrollNodes.Length; i++)
                try { _scrollNodes[i] = GetNode<Node2D>(Scroll_Dialouge_Script.ScrollNames[i]); } catch { }

            _spellbookUIID   = Bridge.GetNode("Spellbook");
            _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            _enemyInfoID        = Bridge.GetNode("EnemyInfo");
            _enemyInfoPopupID   = Bridge.GetNode("EnemyInfoPopUp");
            _enemyInfoPopupBgID = Bridge.GetNode("EnemyInfoPopUpBG");

            IsGamePaused = false;
        }

        private void UpdateHealthBar()
        {
            if (_hpBarFull == null) return;

            if (!_hpBarReady)
            {
                float w = _hpBarFull.width;
                if (w <= 0f) return;
                _hpBarInitW = w;
                _hpBarInitH = _hpBarFull.height;

                // Capture initial canvas-local offset so we can restore it each frame
                if (_canvas != null)
                {
                    _hpBarLocalX = _hpBarFull.position.x - Bridge.GetPositionX(_canvas.ID);
                    _hpBarLocalY = _hpBarFull.position.y - Bridge.GetPositionY(_canvas.ID);
                }

                _hpBarReady = true;
            }

            float pct  = _player.maxHealth > 0f ? _player.health / _player.maxHealth : 0f;
            float newW = _hpBarInitW * pct;

            // Shift the bar center left so the left edge stays fixed while the right edge shrinks
            float offsetX = (_hpBarInitW - newW) * 0.5f;
            Bridge.SetLocalPosition(_hpBarFull.ID, _hpBarLocalX - offsetX, _hpBarLocalY);
            _hpBarFull.setSize(newW, _hpBarInitH);
        }

        public void Update()
        {
            // P key toggles in-game pause
            if (Input.isKeyPressed(KeyCode.P))
            {
                IsGamePaused = !IsGamePaused;
                Bridge.SetCanvasActive(_winLoseCanvasID, IsGamePaused);
                uint buttonID = Bridge.GetNode("PauseScreen");
                Bridge.SetVisibility(buttonID, true);
                SetChildButtonsActive(buttonID, IsGamePaused);
                Bridge.SetAudioPaused(IsGamePaused);
            }
            if (IsGamePaused) return;

            UpdateHealthBar();

            // Freeze player and skip input during dialogue
            if (Scroll_Dialouge_Script.DialogueActive)
            {
                _player.velocity = new Vector2(0f, 0f);
                return;
            }

            // F key — toggle spellbook UI
            if (Input.isKeyPressed(KeyCode.F) && _spellbookUIID != 0)
            {
                _spellbookOpen = !_spellbookOpen;
                Bridge.SetVisibility(_spellbookUIID, _spellbookOpen);
            }

            if (_isAttacking || _isHurt) return;

            float dt = Time.deltaTime;

            // Build direction vector from held keys
            float dx = 0f;
            float dy = 0f;

            if (Input.isKeyHeld(KeyCode.W)) dy -= 1f;
            if (Input.isKeyHeld(KeyCode.S)) dy += 1f;
            if (Input.isKeyHeld(KeyCode.A)) dx -= 1f;
            if (Input.isKeyHeld(KeyCode.D)) dx += 1f;

            // Normalize diagonal movement so speed is consistent in all directions
            if (dx != 0f && dy != 0f)
            {
                float inv = 1f / MathF.Sqrt(dx * dx + dy * dy);
                dx *= inv;
                dy *= inv;
            }

            // E key — close enemy info panel if open
            if (_enemyInfoOpen && Input.isKeyPressed(KeyCode.E))
            {
                _enemyInfoOpen = false;
                if (_enemyInfoID      != 0) Bridge.SetVisibility(_enemyInfoID, false);
                if (_enemyInfoPopupID != 0)
                {
                    Bridge.SetVisibility(_enemyInfoPopupID, false);
                    Bridge.SetButtonActive(_enemyInfoPopupID, false);
                }
                if (_enemyInfoPopupBgID != 0) Bridge.SetVisibility(_enemyInfoPopupBgID, false);
                return;
            }

            // Check scroll interaction before applying movement
            if (Input.isKeyPressed(KeyCode.E))
            {
                for (int i = 0; i < _scrollNodes.Length; i++)
                {
                    if (_scrollNodes[i] != null && Bridge.IsColliding(_scrollNodes[i]!.ID, _player.ID))
                    {
                        Scroll_Dialouge_Script.TriggerDialogue(i);
                        Scroll_Dialouge_Script.TriggerNarrative(i);
                        return;
                    }
                }
            }

            _player.velocity = new Vector2(dx * speed, dy * speed);

            // Flip sprite horizontally based on horizontal direction
            if (dx != 0f)
            {
                Vector2 s = _player.scale;
                _player.scale = new Vector2(dx > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
            }
        }
    
        // Called when entering the idle state
        public void Idle_enter()
        {
            _player.setAnimationLooping(true);
        }

        // Called every frame while in the idle state
        public void Idle_update()
        {
            if (IsMoving())
                _player.changeState("Walk");
        }

        // Called when exiting the idle state
        public void Idle_exit()
        {
            // TODO: Add exit logic for idle state
        }

        // Called when entering the walk state
        public void Walk_enter()
        {
            _player.setAnimationLooping(true);
        }

        // Called every frame while in the walk state
        public void Walk_update()
        {
            if (!IsMoving())
                _player.changeState("Idle");
        }

        // Called when exiting the walk state
        public void Walk_exit()
        {
            // TODO: Add exit logic for walk state
        }

        private void SetChildButtonsActive(uint parentID, bool active)
        {
            uint count = Bridge.GetChildCount(parentID);
            for (uint i = 0; i < count; i++)
            {
                uint childID = Bridge.GetChild(parentID, i);
                if (Bridge.GetNodeType(childID) == (uint)NodeType.BUTTON)
                    Bridge.SetButtonActive(childID, active);
                SetChildButtonsActive(childID, active);
            }
        }

        private bool IsMoving()
        {
            return Input.isKeyHeld(KeyCode.W) || Input.isKeyHeld(KeyCode.S) ||
                   Input.isKeyHeld(KeyCode.A) || Input.isKeyHeld(KeyCode.D);
        }
    
        public void Cast_enter()
        {
            _player.animation = "Cast";
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(castDuration / frames);
        }

        public void Cast_update()
        {
            if (IsMoving())
                _player.changeState("CastWalk");
        }

        public void Cast_exit()
        {
        }
    
        public void CastWalk_enter()
        {
            _player.setAnimationLooping(true);
        }

        public void CastWalk_update()
        {
            if (!IsMoving())
                _player.changeState("Cast");
        }

        public void CastWalk_exit()
        {
        }
    
        public void Attack_enter()
        {
            _isAttacking = true;
            _attackTimer = 0f;
            _player.velocity = new Vector2(0f, 0f);
            _player.animation = "Attack";
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(attackDuration / frames);
        }

        public void Attack_update()
        {
            _attackTimer += Time.deltaTime;
            if (_attackTimer >= attackDuration)
                _player.changeState("Idle");
        }

        public void Attack_exit()
        {
            _isAttacking = false;
        }
    
        public void Hurt_enter()
        {
            _isHurt    = true;
            _hurtTimer = 0f;
            _player.velocity  = new Vector2(0f, 0f);
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(hurtDuration / frames);
        }

        public void Hurt_update()
        {
            _hurtTimer += Time.deltaTime;
            if (_hurtTimer >= hurtDuration)
                _player.changeState(IsMoving() ? "Walk" : "Idle");
        }

        public void Hurt_exit()
        {
            _isHurt = false;
        }
    
        public void Death_enter()
        {
            _player.stateChangeLocked = true;
            _player.velocity = new Vector2(0f, 0f);
        }

        public void Death_update()
        {
            // TODO: Add update logic for Death state
        }

        public void Death_exit()
        {
            // TODO: Add exit logic for Death state
        }
    
		public void _when_enemyinfopopup_onclick()
        {
            if (_enemyInfoID != 0)
            {
                _enemyInfoOpen = true;
                Bridge.SetVisibility(_enemyInfoID, true);
            }
        }
    }
}
