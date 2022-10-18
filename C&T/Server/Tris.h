class Tris{
	private :
		int Box[9];
		int Turno;
		int QuanteMosse;
	
	public :
		Tris()
		{
			for(int i=0;i<9;i++) Box[i] = 0;
			Turno = 1;
			QuanteMosse = 0;
		}
		
		bool InserisciSegno(int Dove)
		{
			bool done = false;
			
			if(Box[Dove] == 0)
			{
				Box[Dove] = Turno;
				Turno = 3-Turno;
				QuanteMosse++;
				done = true;
			}
			
			return done;
		}
		
		bool Vincitore()
		{
			return (Box[0] & Box[3] & Box[6] | Box[1] & Box[4] & Box[7]  | Box[2] & Box[5] & Box[8] | Box[0] & Box[1] & Box[2] | Box[3] & Box[4] & Box[5] | Box[6] & Box[7] & Box[8] | Box[0] & Box[4] & Box[8] | Box[2] & Box[4] & Box[6]);
		}
		
		bool ContinuaPartita()
		{
			bool answer = true;

			if(QuanteMosse>8)
				answer = false;
				
			return answer;
		}
		
		string DaiCampo()
		{
			string Risposta;

			for(int i=0;i<9;i++)
				if(Box[i] == 1)
					Risposta.push_back('X');
				else if(Box[i] == 2)
					Risposta.push_back('O');
				else
					Risposta.push_back(' ');
			
			return Risposta;
		}
};
