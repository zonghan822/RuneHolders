namespace Eno
{
    public class CheatManager : ScriptBase
    {
        public void Begin() { }

        public void Update()
        {
            bool f1 = Input.isKeyPressed(KeyCode.F1);
            bool bumpers = Input.isGamepadButtonHeld(GamepadButton.LeftBumper)
                        && Input.isGamepadButtonPressed(GamepadButton.RightBumper);

            if (!f1 && !bumpers) return;

            foreach (Enemy enemy in GetAllEnemies())
            {
                enemy.health = 0f;
            }
        }

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
    }
}
