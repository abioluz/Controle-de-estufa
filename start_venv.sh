#! /bin/bash
echo "#################################################"
echo "#         Instalando Ambiente Virtual           #"
echo "#################################################"
echo
python3 -m venv venv;
echo
echo "#################################################"
echo "#         Instalando Requerimentos              #"
echo "#################################################"
echo
source venv/bin/activate;
pip3 install -r requirements.txt
echo
echo "Aperte ENTER para sair"
echo
read;


