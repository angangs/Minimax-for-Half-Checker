#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <list>
#include <iostream>
using namespace std;


/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "Achilles";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/

/****************************************************ADDED BY ANXEL******************************************************/
Move MiniMax(int depth);	// main function for MiniMax algorithm
int MinMove(int depth);	// helper function for MiniMax algorithm
int MaxMove(int depth);	// helper function for MiniMax algorithm

Move minimaxAlphaBeta (int alpha, int beta,int depth);
int alphaBetaMin(int alpha, int beta,int depth );
int alphaBetaMax(int alpha, int beta,int depth );

int evaluationFun();

void generate_moves(std::list<Move> &move_list);// helper function for MiniMax algorithm
void generateJumpMove(std::list<Move> &move_list,int i,int j,int k);// helper function for generate_move function

int terminalTest(int depth);// helper function for generate_moves function

bool jumpAble();
void undoMove(Move * moveToDo );

/*
void printMove(Move *moveToPrint){
	int j=0;
	cout<<"\nMove: ";
	while(moveToPrint->tile[0][j]!=-1){
		cout<<(char)(((int)'0')+((int)moveToPrint->tile[0][j]))<<","<<(char)(((int)'0')+((int)moveToPrint->tile[1][j]));
		if( j + 1 < MAXIMUM_MOVE_SIZE ){
			if( moveToPrint->tile[ 0 ][ j + 1 ] != -1 )
				cout << " -> ";
		}
		else{
			break;
		}
			
		j++;
	}
	cout << "\n";
}
*/

/*
 * V(state) = | my( 1400 * kings + 1200 * side_pieces + 1000 * simple_pieces) |  - | opponent's( 1400 * kings + 1200 * side_pieces + 1000 * simple_pieces) |
 */
int evaluationFun(){
	int myCounterPieces=0,opponentCounterPieces=0,myKings=0,opponentKings=0,myCornerCounterPieces=0,opponentCornerCounterPieces=0;
	int i=0,j=0;
	int abs1=0,abs2=0;
	int evaluation=0;
	for(i = 0; i < BOARD_SIZE; i++ ){///////////////evaluate pieces
		for(j = 0; j < BOARD_SIZE; j++ ){
			if(myColor==WHITE){
				if( ( gamePosition.board[ i ][ j ] == myColor ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					myCornerCounterPieces++;
				}
				else if( ( gamePosition.board[ i ][ j ] == getOtherSide(myColor) ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					opponentCornerCounterPieces++;
				}
				else if(( gamePosition.board[ i ][ j ] == myColor )){
					myCounterPieces++;
				}
				else if(( gamePosition.board[ i ][ j ] == getOtherSide(myColor) )){
					opponentCounterPieces++;
				}
			}
			else if(myColor==BLACK){
				if( ( gamePosition.board[ i ][ j ] == myColor ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					myCornerCounterPieces++;
				}
				else if( ( gamePosition.board[ i ][ j ] == getOtherSide(myColor) ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					opponentCornerCounterPieces++;
				}
				else if(( gamePosition.board[ i ][ j ] == myColor )){
					myCounterPieces++;
				}
				else if(( gamePosition.board[ i ][ j ] == getOtherSide(myColor) )){
					opponentCounterPieces++;
					
				}
			}
		}
	}
	abs1=myCounterPieces*1000+myCornerCounterPieces*1200;
	abs2=opponentCounterPieces*1000+opponentCornerCounterPieces*1200;	
	evaluation=abs1-abs2 + ( gamePosition.score[ myColor ] * 1400) - ( gamePosition.score[ getOtherSide( myColor ) ] * 1400);
		//cout<<"\n\nabs1: "<<abs1<<" - abs2: "<<abs2<<" = "<<evaluation<<"\n";	
	return evaluation;
}
/*
 *Prosdiorizei pote termatizei to paixnidi,
 * oi katastaseis stis opoies to paixnidi exei teleiwsei
 * onomazontai termatikes (Terminal states)
 */

int terminalTest(int depth){
	int x = evaluationFun();
	//cout<<"\n\ndepth is: "<<depth;
	return (10/(2+8*exp(x/5)))+10;
}

void undoMove(Move * moveToDo )
{
	int j;
	int intex = 0;
	int stepI, stepJ;

	while( moveToDo->tile[ 0 ][ intex ] != -1 )		//while we have tile available
	{
		intex++;
		if( intex == MAXIMUM_MOVE_SIZE )	// if all move tiles used
			break;
	}
	intex--;
	while(intex>0){
		gamePosition.board[ moveToDo->tile[ 0 ][ intex - 1 ] ][ moveToDo->tile[ 1 ][ intex - 1 ] ] = moveToDo->color;	//place piece
		
		if( abs( moveToDo->tile[ 0 ][ intex - 1 ] - moveToDo->tile[ 0 ][ intex ] ) > 1 ){	//if we had jump
			stepI = ( moveToDo->tile[ 0 ][ intex ] - moveToDo->tile[ 0 ][ intex - 1 ] ) / 2;
			stepJ = ( moveToDo->tile[ 1 ][ intex ] - moveToDo->tile[ 1 ][ intex - 1 ] ) / 2;
			gamePosition.board[ moveToDo->tile[ 0 ][ intex - 1 ] + stepI ]
				[ moveToDo->tile[ 1 ][ intex - 1 ] + stepJ ] = getOtherSide(moveToDo->color);
			//remove the captured piece
		}
		
		if( moveToDo->tile[ 0 ][ intex ] == 0 )
			gamePosition.score[ BLACK ]--;	//Black scored!
		else if( moveToDo->tile[ 0 ][ intex ] == BOARD_SIZE - 1 )
			gamePosition.score[ WHITE ]--;	//White scored!
		else
			gamePosition.board[ moveToDo->tile[ 0 ][ intex ] ][ moveToDo->tile[ 1 ][ intex ] ] = EMPTY;	//remove piece

		intex--;
	}
		
	/*change turn*/
	gamePosition.turn = getOtherSide( gamePosition.turn );

}

/*
 * 	Check if there is "jump" availability
 */
bool jumpAble(){
	for( int i = 0; i < BOARD_SIZE; i++ )
	{
		for( int j = 0; j < BOARD_SIZE; j++ )
		{
			if( gamePosition.board[ i ][ j ] == gamePosition.turn )
			{
				if( canJump( i, j, gamePosition.turn, &gamePosition )!=0 )
					return true;
			}
		}
	}
	return false;
}

/*
 *  GENERATE A LIST OF TOTAL MOVEMENTS
 */

void generate_moves(std::list<Move> &move_list) {
	int playerDirection;
	if( !jumpAble() ){
		for( int i = 0; i < BOARD_SIZE; i++ ){
			for( int j = 0; j < BOARD_SIZE; j++ ){				
				if( gamePosition.board[ i ][ j ] == gamePosition.turn ){	//find a piece of ours
					
					if( gamePosition.turn == WHITE)
						playerDirection=1;
					else
						playerDirection=-1;
					
					myMove.color=gamePosition.turn;
					myMove.tile[ 0 ][ 0 ] = i;		//piece we are going to move
					myMove.tile[ 1 ][ 0 ] = j;
					myMove.tile[ 0 ][ 1 ] = i + 1 * playerDirection;
					myMove.tile[ 0 ][ 2 ] = -1;
					
					myMove.tile[ 1 ][ 1 ] = j - 1;
					
					if( isLegal( &gamePosition, &myMove ))
						move_list.push_back(myMove);
					
					myMove.tile[ 1 ][ 1 ] = j + 1;
					
					if( isLegal( &gamePosition, &myMove ))
						move_list.push_back(myMove);	
				}
			}
		}
	}
	else{
		for( int i = 0; i < BOARD_SIZE; i++ ){
			for( int j = 0; j < BOARD_SIZE; j++ ){				
				if( gamePosition.board[ i ][ j ] == gamePosition.turn ){
					if( canJump( i, j, gamePosition.turn, &gamePosition ) )
							generateJumpMove(move_list,i,j,1);
				}
			}
		}
	}
	
	if(move_list.empty()) {
		myMove.tile[ 0 ][ 0 ] = -1; 
		move_list.push_back(myMove);
	}
}
/*
 * Recursive move generator for jump
 */
void generateJumpMove(std::list<Move> &move_list,int i,int j,int k){
	std::list<Move>::iterator it;
	int playerDirection;
	if( canJump( i, j, gamePosition.turn, &gamePosition ) == 0){	//maximum tiles reached
		move_list.push_back(myMove);
		return;
	}	
	if( ( (  canJump( i, j, gamePosition.turn, &gamePosition )  ) % 2 ) == 1 ){	//if left jump possible
		
		if( gamePosition.turn == WHITE)
			playerDirection=1;
		else
			playerDirection=-1;
		myMove.color=gamePosition.turn;
		myMove.tile[ 0 ][ k-1 ]=i;
		myMove.tile[ 1 ][ k-1 ]=j;
		myMove.tile[ 0 ][ k ] = i + 2 * playerDirection;
		myMove.tile[ 1 ][ k ] = j - 2;
		
		if( !(k+1 > 3 ) )
			myMove.tile[ 0 ][ k+1 ] = -1;
		
		generateJumpMove(move_list,myMove.tile[ 0 ][ k ],myMove.tile[ 1 ][ k ],k+1);	
	}
	if( (  canJump( i, j, gamePosition.turn, &gamePosition )  )  >  1 ){		//right jump possible
		
		if( gamePosition.turn == WHITE)
			playerDirection=1;
		else
			playerDirection=-1;
		myMove.color=gamePosition.turn;
		myMove.tile[ 0 ][ k-1 ]=i;
		myMove.tile[ 1 ][ k-1 ]=j;
		myMove.tile[ 0 ][ k ] = i + 2 * playerDirection;
		myMove.tile[ 1 ][ k ] = j + 2;
		
		if( !(k+1 > 3 ) )
			myMove.tile[ 0 ][ k+1 ] = -1;

		generateJumpMove(move_list,myMove.tile[ 0 ][ k ],myMove.tile[ 1 ][ k ],k+1);
	}
}

/*
 * MINIMAX
 */
// returns best move for my player

Move MiniMax(int depth ) {

	int best_val = INT_MIN, index = 0,playerDirect,playerDirection;
	std::list<Move> move_list;
	Move best_move[12*4];//12 einai to plh8os twn pouliwn, 
						  //8ewrw oti to ka8ena mporei na kanei 
						  //max 4 diaforetikes kinhseis
	generate_moves( move_list );///////gemizoume th lista move_list
	while(!move_list.empty()) {
		
		doMove(&gamePosition,&(move_list.front()));

		depth=0;
		
		int val = MinMove(depth+1);////min turn
		
		if(val > best_val) {/////////////////max turn
			best_val = val;
			index = 0;
			best_move[index]= move_list.front();
		}
		else if(val == best_val)
			best_move[++index] = move_list.front();

		undoMove(&(move_list.front()));

		move_list.pop_front();
	}
	
	if(index > 0)
		index = (rand() % index + 1);
			
	return best_move[index];
}

/*
 * MINMOVE
 */
// finds best move for min player
int MinMove(int depth) {

	if(depth>=terminalTest(depth) || (!canMove( &gamePosition, myColor ) && !canMove( &gamePosition, getOtherSide(myColor) ))) {
		return evaluationFun();
	}

	int best_val = INT_MAX;
	std::list<Move> move_list;

	generate_moves(move_list);
	
	while(!move_list.empty()) {

		doMove(&gamePosition,&(move_list.front()));

		int val = MaxMove(depth+1);

		if(val < best_val) {
			best_val = val;
		}
		undoMove(&(move_list.front()));

		move_list.pop_front();
	}
	return best_val;
}

/*
 * MAXMOVE
 */
// finds best move for max player
int MaxMove(int depth) {
	if(depth>=terminalTest(depth) || (!canMove( &gamePosition, myColor ) && !canMove( &gamePosition, getOtherSide(myColor) ))) {
		return evaluationFun();
	}

	int best_val = INT_MAX;
	std::list<Move> move_list;

	generate_moves(move_list);
	
	while(!move_list.empty()) {
			
		doMove(&gamePosition,&(move_list.front()));
				
		int val = MinMove(depth+1);

		if(val > best_val) {
			best_val = val;
		}
		undoMove(&(move_list.front()));
		move_list.pop_front();
	}
	return best_val;
}
/*
 * 
 * ALPHA-BETA
 * 
 */
 
 /* The Minimax algorithm with Alpha-Beta cutoff */

Move minimaxAlphaBeta (int alpha, int beta,int depth)
{
	int best_val = INT_MIN, index = 0,playerDirect,playerDirection;
	std::list<Move> move_list;
	Move best_move[12*4];//12 einai to plh8os twn pouliwn, 
						  //8ewrw oti to ka8ena mporei na kanei 
						  //max 4 diaforetikes kinhseis
						  
	generate_moves( move_list );///////gemizoume th lista move_list
	while(!move_list.empty()) {
		
		doMove(&gamePosition,&(move_list.front()));

		depth=0;
		
		int val = alphaBetaMin(alpha,beta,depth+1);////min turn

		if(val > best_val) {/////////////////max turn
			best_val = val;
			index = 0;
			best_move[index]= move_list.front();
		}
		else if(val == best_val)
			best_move[++index] = move_list.front();

		undoMove(&(move_list.front()));

		move_list.pop_front();
	}
	
	if(index > 0)
		index = (rand() % index + 1);
	
	return best_move[index];
}

 
int alphaBetaMax(int alpha, int beta,int depth ) {
  	
	if(depth>=terminalTest(depth) || (!canMove( &gamePosition, myColor ) && !canMove( &gamePosition, getOtherSide(myColor) ))) {
		return evaluationFun();
	}

	int best_val = INT_MAX;
	std::list<Move> move_list;

	generate_moves(move_list);
	int l=0;
	
	while(!move_list.empty()) {
				
		doMove(&gamePosition,&(move_list.front()));
		
		int val = alphaBetaMin(alpha,beta,depth+1);

		if(val >= beta) {
			undoMove(&(move_list.front()));
		
			move_list.clear();
			return beta;
		}
		
		if(val>alpha)
			alpha=val;
		
		undoMove(&(move_list.front()));
		
		move_list.pop_front();
	}
	return alpha;
}
 
int alphaBetaMin(int alpha, int beta,int depth ) {
	
	if(depth>=terminalTest(depth) || (!canMove( &gamePosition, myColor ) && !canMove( &gamePosition, getOtherSide(myColor) ))) {
		return evaluationFun();
	}

	int best_val = INT_MAX;
	std::list<Move> move_list;

	generate_moves(move_list);
	
	while(!move_list.empty()) {

		
		doMove(&gamePosition,&(move_list.front()));
		
		int val = alphaBetaMax(alpha,beta,depth+1);
		
		if(val<=alpha){
			undoMove(&(move_list.front()));
			
			move_list.clear();
			return alpha;
		}
		if(val < beta) {
			beta=val;
		}
		undoMove(&(move_list.front()));
		
		move_list.pop_front();
	}
	return beta;
}

/*****************************************************END OF ANXEL ADD*****************************************************/

int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}



	connectToTarget( port, ip, &mySocket );

	char msg;

/**********************************************************/
// used in random
	srand( time( NULL ) );
	int i, j, k;
	int jumpPossible;
	int playerDirection;
/**********************************************************/

	while( 1 )
	{

		msg = recvMsg( mySocket );

		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				getPosition( &gamePosition, mySocket );
				//printPosition( &gamePosition );
				break;

			case NM_COLOR_W:			//server indorms us that we have WHITE color
				myColor = WHITE;
				break;

			case NM_COLOR_B:			//server indorms us that we have BLACK color
				myColor = BLACK;
				break;

			case NM_PREPARE_TO_RECEIVE_MOVE:	//server informs us that he will send opponent's move
				getMove( &moveReceived, mySocket );
				moveReceived.color = getOtherSide( myColor );
				doMove( &gamePosition, &moveReceived );		//play opponent's move on our position
				//printPosition( &gamePosition );
				break;
/********************************************************************************************************************/
			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else
				{
					/*
					 * 
					 * MINIMAX & ALPHA-BETA PRUNING
					 * 
					 */
					//myMove=MiniMax(0);
					myMove=minimaxAlphaBeta(INT_MIN,INT_MAX,0);
					
				}
				sendMove( &myMove, mySocket );			//send our move

				doMove( &gamePosition, &myMove );		//play our move on our position

				break;
/********************************************************************************************************************/
			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	} 

	return 0;
}






