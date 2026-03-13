namespace Eno
{
    public class Room24Scroll_Script : ScriptBase
    {
        public void Begin()
        {
        }

        public void Update()
        {
            uint playerID = Bridge.GetNode("Player");
            if (playerID == 0 || playerID == uint.MaxValue) return;

            if (Bridge.IsColliding(ID, playerID) && Input.isKeyPressed(KeyCode.E))
            {
                SceneManager.changeScene("MainMenu.txt");
            }
        }

        public void Default_enter()
        {
        }

        public void Default_update()
        {
        }

        public void Default_exit()
        {
        }
    }
}
