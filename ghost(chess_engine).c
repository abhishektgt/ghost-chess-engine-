#include <stdio.h>
#include<time.h>
#include <string.h>
#include <windows.h>
#include<stdlib.h>
#define U64 unsigned long long
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard,square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard,square) ((bitboard) &= ~(1ULL << (square)))
// or can use xor but have to check for 1 or 0
// board squares



enum {
    a8, b8, c8, d8, e8, f8, g8, h8,  // 0-7
    a7, b7, c7, d7, e7, f7, g7, h7,  // 8-15
    a6, b6, c6, d6, e6, f6, g6, h6,  // 16-23
    a5, b5, c5, d5, e5, f5, g5, h5,  // 24-31
    a4, b4, c4, d4, e4, f4, g4, h4,  // 32-39
    a3, b3, c3, d3, e3, f3, g3, h3,  // 40-47
    a2, b2, c2, d2, e2, f2, g2, h2,  // 48-55
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq  // 56-63
};


//
enum{
    white, black, both
};
// bishop and rook
enum{
    rook,bishop
};
// enum for castling
enum{
    wk=1,wq=2,bk=4,bq=8
};

// encoding pieces
enum{
    P, N, B, R, Q , K, p ,n, b, r, q, k
};


 ******Board********



// representing the board
U64 bitboards[12];

// occupancy bitboards{white,black, all}
U64 occupancies[3];

// side to move
int side;

// enpassant sqaure
int enpassant = no_sq;

// castling rights
// default value of global variable is zwero
int castle;


// board squares to be used later
// string literal returns a char pointer
const char *square_to_coordinates[]={
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

// ASCII PIECES(how it's working and use)
char ascii_pieces[12]="PNBRQKpnbrqk";

// unicode pieces(how it's working and use)
char *unicode_pieces[12]={"♙","♘","♗","♖","♕","♔","♟","♞","♝","♜","♛","♚"};

// convert ascii characters pieces to encoded constants
int char_pieces[]={
    ['P']=P,
    ['N']=N,
    ['B']=B,
    ['R']=R,
    ['Q']=Q,
    ['K']=K,
    ['p']=p,
    ['n']=n,
    ['b']=b,
    ['r']=r,
    ['q']=q,
    ['k']=k
};


// promoted pieces
char promoted_pieces[]={
    [Q]='q',
    [R]='r',
    [B]='b',
    [N]='n',
    [q]='q',
    [r]='r',
    [b]='b',
    [n]='n'
};
// File masks (needed for the attacks function)
const U64 FILE_A = 0x0101010101010101ULL;
const U64 FILE_B = 0x0202020202020202ULL;
const U64 FILE_G = 0x4040404040404040ULL;
const U64 FILE_H = 0x8080808080808080ULL;



/********************************
 *     Magic Bitboards         *
 *******************************/


//pseudo random number state
unsigned int random_state=1804289383;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number(){
    //get current state
    unsigned int current_state = random_state;
    // xor shift algo to make random number from already present random number
    current_state^= current_state << 13;
    current_state^= current_state >> 17;
    current_state^= current_state << 5;
    //update random number state
    random_state=current_state;
    // return random nnumber
    return current_state;
}
// generate 64 bit pseudo random number
U64 get_random_U64_number(){
    // four random numbers
    U64 n1,n2,n3, n4;
    // init random slicing msb side first 16 bits
    n1=(U64)(get_random_U32_number()) & 0xFFFF;
    n2=(U64)(get_random_U32_number()) & 0xFFFF;
    n3=(U64)(get_random_U32_number()) & 0xFFFF;
    n4=(U64)(get_random_U32_number()) & 0xFFFF;

    // return random number
    return n1| (n2<<16) | (n3<<32) | (n4<<48);
}
// generate magic number candidate
U64 generate_magic_number(){
    return get_random_U64_number()&get_random_U64_number()&get_random_U64_number();
}


// pawn attacks table
U64 pawn_attacks[2][64];

//knight attack tables
U64 knight_attacks[64];

// king attack table
U64 king_attacks[64];

// bishop masks
U64 bishop_masks[64];

// rook masks
U64 rook_masks[64];

// bishop attacks table [square][occupancies]
U64 bishop_attacks[64][512];

// rook attacks table [square][occupancies]
U64 rook_attacks[64][4096];
// bit count of relevant bishop attack using mask_bishop_atack
const int bishop_relevant_bits[] ={
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// bit count of relevant rook attack using mask_bishop_atack
const int rook_relevant_bits[] ={
    12,11,11,11,11,11,11,12,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    12,11,11,11,11,11,11,12

};
// rook magic numbers
U64 rook_magic_numbers[64]={
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};
// bishop magic numbers
U64 bishop_magic_numbers[64]={
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};


// generate pawnn attacks
U64 mask_pawn_attacks(int side, int square){
    U64 bitboard =0ULL;
    U64 attacks =0ULL;
    set_bit(bitboard,square);
    // white side
    if(side==0){
        if(square%8==7){
            attacks |= (bitboard >> 9);

        }
        else if(square%8==0){
            attacks |= (bitboard >> 7);
        }
        else{
            attacks |= (bitboard >> 9);
            attacks |= (bitboard >> 7);
        }
    }
    else{
        if(square%8==7){
            attacks |= (bitboard << 7);
        }
        else if(square%8==0){
            attacks |= (bitboard << 9);
        }
        else{
            attacks |= (bitboard << 9);
            attacks |= (bitboard << 7);
        }
    }
    return attacks;
}
// mask knight attacks

U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;

    // Place the knight on the given square
    set_bit(bitboard, square);

    // Generate attacks
    if ((bitboard >> 17) & ~FILE_H) attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & ~FILE_A) attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & ~(FILE_G | FILE_H)) attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & ~(FILE_A | FILE_B)) attacks |= (bitboard >> 6);

    if ((bitboard << 17) & ~FILE_A) attacks |= (bitboard << 17);
    if ((bitboard << 15) & ~FILE_H) attacks |= (bitboard << 15);
    if ((bitboard << 10) & ~(FILE_A | FILE_B)) attacks |= (bitboard << 10);
    if ((bitboard << 6) & ~(FILE_G | FILE_H)) attacks |= (bitboard << 6);

    return attacks;
}
// masking king attacks
U64 mask_king_attacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;

    // Place the king on the given square
    set_bit(bitboard, square);

    // Generate attacks
    if ((bitboard >> 1) & ~FILE_H) attacks |= (bitboard >> 1);
    if ((bitboard >> 9) & ~FILE_H) attacks |= (bitboard >> 9);
    if ((bitboard >> 8)) attacks |= (bitboard >> 8);
    if ((bitboard >> 7) & ~FILE_A) attacks |= (bitboard >> 7);
    if ((bitboard << 1) & ~FILE_A) attacks |= (bitboard << 1);
    if ((bitboard << 9) & ~FILE_A) attacks |= (bitboard << 9);
    if ((bitboard << 8)) attacks |= (bitboard << 8);
    if ((bitboard << 7) & ~FILE_H) attacks |= (bitboard << 7);

    return attacks;
}
// masking bishop attacks
U64 mask_bishop_attacks(int square){
    U64 attacks = 0ULL;

    // Generate attacks
    int rank,file;
    int trank,tfile;
    trank=square/8;
    tfile=square%8;
    for(int rank=trank+1, file=tfile+1; rank<=6 && file<=6; ++rank, ++file){
        set_bit(attacks, rank*8+file);
    }
    for(int rank=trank-1, file=tfile-1; rank>=1 && file>=1; --rank, --file){
        set_bit(attacks, rank*8+file);
    }
    for(int rank=trank+1, file=tfile-1; rank<=6 && file>=1; ++rank, --file){
        set_bit(attacks, rank*8+file);
    }
    for(int rank=trank-1, file=tfile+1; rank>=1 && file<=6; --rank, ++file){
        set_bit(attacks, rank*8+file);
    }

    return attacks;

}
// masking rook attacks
U64 mask_rook_attacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;

    // Place the rook on the given square
    set_bit(bitboard, square);

    // Generate attacks
    int rank,file;
    int trank,tfile;
    trank=square/8;
    tfile=square%8;
    for(int rank=trank+1, file=tfile; rank<=6; ++rank){
        set_bit(attacks, rank*8+file);
    }
    for(int rank=trank-1, file=tfile; rank>=1; --rank){
        set_bit(attacks, rank*8+file);
    }
    for(int rank=trank, file=tfile+1; file<=6; ++file){
        set_bit(attacks, rank*8+file);
    }
    for(int rank=trank, file=tfile-1; file>=1; --file){
        set_bit(attacks, rank*8+file);
    }

    return attacks;

}
// bishop attacks when it is blocked by some
U64 bishop_attacks_fly(int square, U64 block){
    U64 attacks = 0ULL;

    // Generate attacks
    int rank,file;
    int trank,tfile;
    trank=square/8;
    tfile=square%8;
    for(int rank=trank+1, file=tfile+1; rank<=7 && file<=7; ++rank, ++file){
        set_bit(attacks, rank*8+file);
        if((1ULL << (rank*8+file)) & block)
            break;
    }
    for(int rank=trank-1, file=tfile-1; rank>=0 && file>=0; --rank, --file){
        set_bit(attacks, rank*8+file);
        if((1ULL << (rank*8+file)) & block)
            break;
    }
    for(int rank=trank+1, file=tfile-1; rank<=7 && file>=0; ++rank, --file){
        set_bit(attacks, rank*8+file);
        if((1ULL << (rank*8+file)) & block)
            break;
    }
    for(int rank=trank-1, file=tfile+1; rank>=0 && file<=7; --rank, ++file){
        set_bit(attacks, rank*8+file);
        if((1ULL << (rank*8+file)) & block)
            break;
    }

    return attacks;

}
// rook attacks when itis blocked by some
U64 rook_attacks_fly(int square,U64 block){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;

    // Place the rook on the given square
    set_bit(bitboard, square);

    // Generate attacks
    int rank,file;
    int trank,tfile;
    trank=square/8;
    tfile=square%8;
    for(int rank=trank+1, file=tfile; rank<=7; ++rank){
        set_bit(attacks, rank*8+file);
        if(get_bit(block, rank*8+file))
            break;

    }
    for(int rank=trank-1, file=tfile; rank>=0; --rank){
        set_bit(attacks, rank*8+file);
        if(get_bit(block, rank*8+file))
            break;

    }
    for(int rank=trank, file=tfile+1; file<=7; ++file){
        set_bit(attacks, rank*8+file);
        if(get_bit(block, rank*8+file))
            break;
    }
    for(int rank=trank, file=tfile-1; file>=0; --file){
        set_bit(attacks, rank*8+file);
        if(get_bit(block, rank*8+file))
            break;
    }

    return attacks;

}
// counting the no. of bits in a bitboard
static inline int count_bits(U64 b){
    int count =0;
    while(b){
        count++;
        b &= b-1;
    }
    return count;
}
// get the index of least significant bit
static inline int get_lsb_index(U64 b){
    if(b){
        return count_bits((b&-b)-1);
    }
    return -1;
}
// printing the bitboard
void print_bitboard(U64 bitboard){
    for(int rank=1; rank<=8; ++rank){
        for(int file=1; file<=8; ++file){
            int square = (rank-1)*8+file-1;
            if(file==1){
                printf("%d ",9-rank);
            }
            printf("%d ",get_bit(bitboard, square)?1:0);
        }
        printf("\n");
    }
    // print files
    printf("  ");
    for(int i=0; i<8; ++i){
        printf("%c ", 'a'+i);
    }
    printf("\n\n");
    printf("   Bitboard: %llud\n\n", bitboard);
    printf("\n");
}

// print board now a global variable
void print_board(){
    // over ranks
    for(int rank=0; rank<8; ++rank){
        for(int file=0; file<8; ++file){
            //init square
            int square = rank*8+file;
            if(file==0){
                printf("%d ",8-rank);
            }
            // define piece variable
            int piece = -1;

            for(int bb_pieces = P;  bb_pieces<=k; ++bb_pieces){
                if(get_bit(bitboards[bb_pieces], square)){
                    piece = bb_pieces;
                }
            }

            printf(" %c", (piece==-1)? '.' : ascii_pieces[piece]);

        }
        printf("\n");
    }
    printf("\n   ");
    for(int i=0; i<8; ++i){
        printf("%c ", 'a'+i);
    }
    // print side to move
    printf("\n\n   Side:     %s\n", side==white?"white":"black");

    // print enpassant
    printf("   Enpassant:   %s\n",enpassant!=no_sq? square_to_coordinates[enpassant]: "no");

    // prinf castling rights
    printf("   castling:  %c%c%c%c\n\n",(castle&wk)?'K':'-',(castle&wq)?'Q':'-',(castle&bk)?'k':'-',(castle&bq)?'q':'-');
}


// parse FEN strings
void parse_fen(char *fen) {
    // Reset boards (bitboards)
    memset(bitboards, 0ULL, sizeof(bitboards));

    // Reset occupancies
    memset(occupancies, 0ULL, sizeof(occupancies));

    // Reset game state variables
    side = 0;
    enpassant = no_sq;
    castle = 0;

    // Loop over all board squares
    for (int rank = 0; rank <= 7; ++rank) {
        for (int file = 0; file < 8; ++file) {
            // Initialize current square
            int square = rank * 8 + file;

            // Matching ASCII pieces with the FEN string given
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
                // Initialize piece type
                int piece = char_pieces[*fen];
                // Set piece on corresponding bitboard
                set_bit(bitboards[piece], square);
                fen++;
            }
            else if (*fen >= '0' && *fen <= '9') {
                // Initialize empty squares
                int offset = *fen - '0';
                file += offset - 1;
                fen++;
            }

            // Handle rank delimiter
            if (*fen == '/') {
                fen++;
            }
        }
    }
    fen++;
    side = (*fen == 'w') ? white : black;
    fen+=2;
    // there is space
    // Parse castling rights
    while (*fen != ' ') {
        switch (*fen) {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            default: break;
        }
        fen++;
    }
    // dealing in enpassant square now
    fen++;
    if (*fen != '-') {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        enpassant = rank * 8 + file;
    } else {
        enpassant = no_sq;
    }

    // Parse halfmove clock and fullmove number
    // Note: These are usually stored in additional variables for game state
    // but are not handled in this function as given.
    for(int piece=P; piece<=K; ++piece){
        occupancies[white] |= bitboards[piece];
        occupancies[both] |= bitboards[piece];
    }
    for(int piece=p; piece<=k; ++piece){
        occupancies[black] |= bitboards[piece];
        occupancies[both] |= bitboards[piece];

    }

}
// init leaper piece attacks(fixed distance pieces)
void init_leaper_pieces(){
    for(int square=0; square<64; ++square){
        pawn_attacks[white][square]=mask_pawn_attacks(white, square);
        pawn_attacks[black][square]=mask_pawn_attacks(black, square);
    }
    for(int square=0; square<64; ++square){
        knight_attacks[square]=mask_knight_attacks(square);
    }
    for(int square=0; square<64; ++square){
        king_attacks[square]=mask_king_attacks(square);
    }
}
// set occupencies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask){
    U64 occupancy = 0ULL;
    for(int count=0; count<bits_in_mask; ++count){
        int square = get_lsb_index(attack_mask);
        pop_bit(attack_mask, square);
        if(index & (1<<count)){
            set_bit(occupancy, square);
        }
    }
    return occupancy;
}
/************************
 * Magics      *
 ***********************/
// find appropriate magic number
U64 find_magic_number(int square, int relevant_bits, int bishop){
    // init occupancies;
    U64 occupancy[4096];
    // init attacks
    U64 attacks[4096];
    // init used attacks
    U64 used_attacks[4096];

    // attack mask
    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    // 2^12=4096= maxm relevant bits in case of bishop or rook, can be contained in the int
    int occupancy_size = 1 << relevant_bits;
    for(int index = 0; index < occupancy_size; ++index){
        // init occupancy
        occupancy[index] = set_occupancy(index, relevant_bits, attack_mask);
        // init attacks
        attacks[index] = bishop ? bishop_attacks_fly(square, occupancy[index]) : rook_attacks_fly(square, occupancy[index]);
    }
    // testing the magic number

    for(int count = 0; count < 100000000; ++count){
        // generate magic number
        U64 magic_number = generate_magic_number();
        // skip inappropriate magic numbers
        if(count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6){
            continue;
        }
        // init used attacks

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        // init index and fail flag
        int index, fail;

        // test magic index loop
        for(index = 0, fail = 0; fail == 0 && index < occupancy_size; ++index){
            // get the index
            int magic_index = (int) ((occupancy[index] * magic_number) >> (64 - relevant_bits));
            // check if the index is already used
            if(used_attacks[magic_index] == 0ULL){
                used_attacks[magic_index] = attacks[index];
            }
            else if(used_attacks[magic_index] != attacks[index]){
                fail = 1;
            }
        }
        // if no fail return magic number
        if(fail == 0){
            return magic_number;
        }
    }
    // return 0 if no magic number found
    printf("Magic number not found for square %d\n", square);
    return 0ULL;
}

// init magic numbers for all squares
void init_magic_numbers(){
    // loop over all squares
    for(int square = 0; square < 64; ++square){
        // for rook
        rook_magic_numbers[square]=find_magic_number(square, rook_relevant_bits[square], rook);
        //try to understand this , why this particularly works 0x%llxULL
    }
    printf("\n\n");
    for(int square = 0; square < 64; ++square){
        // for bishop
        bishop_magic_numbers[square]=find_magic_number(square, bishop_relevant_bits[square], bishop);
        //try to understand this , why this particularly works 0x%llxULL
    }
}

// init slider piece attack tables
void init_slider_attacks(int bishop){
    // loop over 64 squares
    for(int square=0; square<64; ++square){

        // bishop masking without blocking
        bishop_masks[square]=mask_bishop_attacks(square);

        // rook masking without blocking
        rook_masks[square]=mask_rook_attacks(square);

        // masking attacks
        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

        // relevant bits
        int relevant_bits=count_bits(attack_mask);

        // occupancy size according to relevant bits
        int occupation_size = 1 << relevant_bits;

        // loop through all the possible occupencies
        for(int index=0; index<occupation_size; ++index){
            if(bishop){
                // setting up occupency acc to index
                U64 occupency=set_occupancy(index, relevant_bits, attack_mask);

                // setting up magic index
                int magic_index=(int)((occupency*bishop_magic_numbers[square])>>(64-bishop_relevant_bits[square]));
                ////can directly write relevant_bits

                // setting up bishop attacks
                bishop_attacks[square][magic_index]=bishop_attacks_fly(square, occupency);
            }
            // rook
            else{
                // setting up occupency acc to index
                U64 occupency=set_occupancy(index, relevant_bits, attack_mask);

                // setting up magic index
                int magic_index=(int)((occupency*rook_magic_numbers[square])>>(64-rook_relevant_bits[square]));
                //can directly write relevant_bits

                // setting up rook attacks
                rook_attacks[square][magic_index]=rook_attacks_fly(square, occupency);
            }
        }

    }
}
// get bishop attacks
static inline U64 get_bishop_attacks(int square, U64 occupancy){
    // get magic index
    occupancy &= bishop_masks[square];
    // why this works
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64-bishop_relevant_bits[square];

    return bishop_attacks[square][occupancy];

}
// get rook attacks
static inline U64 get_rook_attacks(int square, U64 occupancy){
    // get magic index
    occupancy &= rook_masks[square];
    // why this works
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64-rook_relevant_bits[square];

    return rook_attacks[square][occupancy];

}
// get queen attacks
static inline U64 get_queen_attacks(int square, U64 occupancy){
    // init result attack bitboard
    U64 queen_attacks=0ULL;

    // init bishop and rook occupancies
    U64 bishop_occupancy=occupancy;
    U64 rook_occupancy=occupancy;

    //get bishops attacks assuming given occupency
    bishop_occupancy &= bishop_masks[square];
    bishop_occupancy *= bishop_magic_numbers[square];
    bishop_occupancy >>= 64-bishop_relevant_bits[square];

    // get rook attacks assuming given occupency
    rook_occupancy &= rook_masks[square];
    rook_occupancy *= rook_magic_numbers[square];
    rook_occupancy >>= 64-rook_relevant_bits[square];

    queen_attacks|= rook_attacks[square][rook_occupancy];
    queen_attacks|= bishop_attacks[square][bishop_occupancy];
    return queen_attacks;
}
    /*****************
     *move generator**
    *****************/


//is current given square attacked by the given side

static inline int is_square_attacked(int square, int side){
    //attacked by white pawns
    if((side==white)&&(pawn_attacks[black][square] & bitboards[P])){
        return 1;
    }

    // attacked by black pawns
    if((side==black)&&(pawn_attacks[white][square] & bitboards[p])){
        return 1;
    }

    // attacked by the knight
    if((knight_attacks[square])& ((side==white)?bitboards[N]:bitboards[n])){
        return 1;
    }

    // if attacked by the bishops
    if(get_bishop_attacks(square,occupancies[both])& ((side==white)?bitboards[B]:bitboards[b])){
        return 1;
    }

    // attacked by the rooks
    if(get_rook_attacks(square,occupancies[both])& ((side==white)?bitboards[R]:bitboards[r])){
        return 1;
    }

    // if attacked by the queen
    if(get_queen_attacks(square,occupancies[both])& ((side==white)?bitboards[Q]:bitboards[q])){
        return 1;
    }

    // attacked by the king
    if((king_attacks[square])& ((side==white)?bitboards[K]:bitboards[k])){
        return 1;
    }

    return 0;// by ddefault
}
//print ttacked sqaure;
void print_attacked_square(int side){
    // loop over ranks and files
    for(int rank=0; rank<8; ++rank){
        for(int file=0; file<8; ++file){
            int square=8*rank+file;
            if(file==0){
                printf("%d  ",8-rank);
            }
            printf("%d ", is_square_attacked(square,side)? 1:0);
        }
        printf("\n");
    }
    printf("\n   ");
    for(int i=0; i<8; ++i){
        printf("%c ", 'a'+i);
    }
}

/*
    // binary move bits                                         // hexadecimal constants
    0000 0000 0000 0000 0011 1111 source square                 0x3f
    0000 0000 0000 1111 1100 0000 target square                 0xfc0
    0000 0000 1111 0000 0000 0000 peice                         0xf000
    0000 1111 0000 0000 0000 0000 promoted peice                0xf0000
    0001 0000 0000 0000 0000 0000 capture flag              0x100000
    0010 0000 0000 0000 0000 0000 double push flag                  0x200000
    0100 0000 0000 0000 0000 0000 enpassant flag                0x400000
    1000 0000 0000 0000 0000 0000 castling flag                 0x800000
*/

#define encode_move(source,target,peice,promoted,capture,double,enpassant,castling) \
    (source)|\
    (target<<6)|\
    (peice<<12)|\
    (promoted<<16)|\
    (capture<<20)|\
    (double<<21)|\
    (enpassant<<22)|\
    (castling<<23)

// extract source square
#define get_move_source(move) (move & 0x3f)

// extract target square
#define get_move_target(move) ((move&0xfc0) >> 6)

//extract peice
#define get_move_peice(move) ((move&0xf000) >> 12)

// extract promoted peice
#define get_move_promoted(move) ((move&0xf0000) >> 16)

// extract capture flag
#define get_move_capture(move) ((move&0x100000))

//extarct double push flag
#define get_move_double(move) ((move&0x200000))

//extract enpassant flag
#define get_move_enpassant(move) ((move&0x400000))

//extract castling flag
#define get_move_castling(move) ((move&0x800000))


// move list struct
typedef struct{
    // moves;
    int moves[256];

    // cnt;
    int count;
}moves;

// add move to the list
static inline void add_move(moves* move_list, int move){
    // store move
    move_list->moves[move_list->count]=move;

    // increment count
    move_list->count++;
}


// print move'(for uci purposes)
void print_move(int move){

    if(get_move_promoted(move)){
        printf("%s%s%c", square_to_coordinates[get_move_source(move)],square_to_coordinates[get_move_target(move)],promoted_pieces[get_move_promoted(move)]);
    }
    else{
        printf("%s%s", square_to_coordinates[get_move_source(move)],square_to_coordinates[get_move_target(move)]);
    }
}

// print move list
void print_move_list(moves *move_list){

    // do nothing on empty move list
    if(!move_list->count){
        printf("\n   no moves to print\n\n");
        return;
    }
    printf("\n   move   piece  capture double enpass castling\n\n");

    // loop over all the moves in the move list
    for(int cnt=0; cnt<move_list->count; ++cnt){
        // init move;
        int move= move_list->moves[cnt];

        printf("   %s%s%c    %c       %d      %d      %d       %d\n\n", square_to_coordinates[get_move_source(move)],
                                                        square_to_coordinates[get_move_target(move)],
                                                        get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)]: ' ',
                                                        ascii_pieces[get_move_peice(move)],
                                                        get_move_capture(move)?1:0,
                                                        get_move_double(move)?1:0,
                                                        get_move_enpassant(move)?1:0,
                                                        get_move_castling(move)?1:0);

    }
    // print total no of moves
        printf("\n\n  Total no. of moves : %d\n\n",move_list->count);
}

// copy board for taking back
#define copy_board()\
    U64 bitboards_copy[12],occupancies_copy[3];\
    int side_copy, enpassant_copy, castle_copy;\
    memcpy(bitboards_copy, bitboards, 96);\
    memcpy(occupancies_copy, occupancies, 24);\
    side_copy=side,enpassant_copy=enpassant, castle_copy=castle;\


// taking back the board
#define take_back()\
    memcpy(bitboards, bitboards_copy, 96);\
    memcpy(occupancies, occupancies_copy, 24);\
    side=side_copy,enpassant=enpassant_copy, castle=castle_copy;\


// move types
enum {
    all_moves,only_captures
};


// castling right update constants
const int castling_rights[64] = {
    7,15,15,15,3,15,15,11,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    13,15,15,15,12,15,15,14
};
// make move on chess board
static inline int make_move(int move, int move_flag){
    // if quiet move

    if(move_flag==all_moves){
        // preserve board state
        copy_board();

        // parse move
        int source_square=get_move_source(move);
        int target_square=get_move_target(move);
        int piece=get_move_peice(move);
        int promoted_piece=get_move_promoted(move);
        int capture=get_move_capture(move);
        int double_push = get_move_double(move);
        int enpass = get_move_enpassant(move);
        int castling= get_move_castling(move);

        // move piece
        pop_bit(bitboards[piece],source_square);
        set_bit(bitboards[piece],target_square);

        // handling capture moves
        if(capture){

             // pick up bitboard piece index ranegs depedning on the side
            int start_piece, end_piece;

            // white to move
            if(side==white){
                start_piece=p;

                end_piece=k;
            }

            //black to move
            else{
                start_piece=P;
                end_piece=K;
            }

            // loop over bitboards opposite to the current side to move
            for(int bb_piece=start_piece; bb_piece<=end_piece; ++bb_piece){
                if(get_bit(bitboards[bb_piece], target_square)){
                    pop_bit(bitboards[bb_piece], target_square);
                    break;
                }
            }
        }

        // handle promotions
        if(promoted_piece){

            //erase the pawn from the target square
            pop_bit(bitboards[(side==white)?P:p], target_square);

            // set up promoed piece on chess board
            set_bit(bitboards[promoted_piece], target_square);
        }

        // enpass mocve
        if(enpass){

            (side==white) ? pop_bit(bitboards[p], target_square+8) : pop_bit(bitboards[P], target_square-8);
        }

        // reset enpas

        enpassant=no_sq;

        // handle double pawn push
        if(double_push){

            // set enpassant square
            (side==white) ? (enpassant=target_square+8):(enpassant=target_square-8);
        }

        // castling move
        if(castling){

            switch(target_square){

                // white castle king side
                case(g1):
                // move  H rook
                pop_bit(bitboards[R],h1);
                set_bit(bitboards[R],f1);
                break;

                // white queen side castling
                case(c1):
                //move A rook
                pop_bit(bitboards[R],a1);
                set_bit(bitboards[R],d1);
                break;

                // black king side castling
                case(g8):
                // move h rook
                pop_bit(bitboards[r],h8);
                set_bit(bitboards[r],f8);
                break;
                // black king queen side castling
                case(c8):

                // move a rook
                pop_bit(bitboards[r],a8);
                set_bit(bitboards[r],d8);
                break;
            }

        }

        //update castling rights

        castle&= castling_rights[source_square];
        castle&= castling_rights[target_square];

        // reset occupencies
        memset(occupancies, 0ULL, 24);

        //loop over white pieces bitboards
        for(int bb_piece=P; bb_piece<=K; ++bb_piece){
            occupancies[white]|= bitboards[bb_piece];
        }

        //loop over black pieces bitboards
        for(int bb_piece=p; bb_piece<=k; ++bb_piece){
            occupancies[black]|= bitboards[bb_piece];
        }
        // update both side occupenecies
        occupancies[both]|=occupancies[white];
        occupancies[both]|=occupancies[black];

        //change sides
        side^=1;

        // make sure that the king is not being attacked
        if(is_square_attacked((side==white)? get_lsb_index(bitboards[k]): get_lsb_index(bitboards[K]),side)){
            // move is illegal take bacl
            take_back();

            return 0;
        }
        else{
            return 1;
        }

    }

    // else capture moves
    else{

        // make sure move is the capture
        if(get_move_capture(move)){
            make_move(move,all_moves);
        }
        // dont return it
        else {
            return 0;
        }
    }

}
// generate all moves
static inline void generate_moves(moves *move_list){
    //init move cnt
    move_list->count=0;

    // define source and target
    int source_square, target_square;

    // define current piece's bitboard copy and it's attacks
    U64 bitboard, attacks;

    // loop over all the bitboards
    for(int piece=P; piece<=k; ++piece){
        //init bitboard to bitboard[p]
        bitboard=bitboards[piece];

        // generate white pawns and white king castling moves
        if(side==white){
            if(piece==P){
                //loop till all the bits are exhausted
                while(bitboard){
                    // init ssource square
                    source_square=get_lsb_index(bitboard);

                    //printf("white pawn: %s", square_to_coordinates[source_square]);

                    // init target square
                    target_square=source_square-8;

                    if(!(target_square<a8) && !(get_bit(occupancies[both], target_square))){

                        // for pawn promotions
                        if(source_square>=a7 && source_square<=h7){

                            // add move to the list

                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                        }
                        // pawn moves without promotion
                        else{
                            // one move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            // two move oush
                            if((source_square>=a2 && source_square<=h2) && !get_bit(occupancies[both], target_square-8)){
                                add_move(move_list, encode_move(source_square, target_square-8, piece, 0, 0, 1, 0, 0));
                            }

                        }

                    }
                    // init attacks
                    attacks= pawn_attacks[side][source_square]& occupancies[black];

                    // loop
                    while(attacks){

                        // init target
                        target_square=get_lsb_index(attacks);

                        // add move to the list
                        // pawn promotion capture
                        if(source_square>=a7 && source_square<=h7){

                            // add move to the list
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));


                        }
                        else{
                            // add oawn capture to the list
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        }
                        pop_bit(attacks, target_square);
                    }
                        // enpassant attack

                    if(enpassant!=no_sq){
                        // enpassant attacks
                        U64 enpassant_attacks= pawn_attacks[side][source_square] & (1ULL << enpassant);

                        //if enpassant attack available
                        if(enpassant_attacks){
                            // add move to the list
                            int target_enpassant= get_lsb_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0 , 1, 0, 1, 0));

                        }

                    }


                    pop_bit(bitboard, source_square);
                }


            }

            // castling movw'
            if(piece==K){
                // king side castling
                if(castle & wk){
                    // check if f1 and g1 are empty
                    if(!(get_bit(occupancies[both], f1)) && !(get_bit(occupancies[both], g1))){

                        //check if e1 and f1 are not attacked by any piece
                        if(!(is_square_attacked(e1, black)) && !(is_square_attacked(f1, black)) ){
                            add_move(move_list, encode_move(e1,g1, piece, 0 , 0, 0, 0 , 1));
                        }

                    }
                }
                // queen side castling
                if(castle & wq){
                    // check if d1 and c1 and b1 are empty
                   if(!(get_bit(occupancies[both], c1)) && !(get_bit(occupancies[both], d1)) && !(get_bit(occupancies[both], b1))){

                        //check if e1 and d1 are not attacked by any piece
                        if(!(is_square_attacked(e1, black)) && !(is_square_attacked(d1, black))){
                            add_move(move_list, encode_move(e1,c1, piece, 0 , 0, 0, 0 , 1));
                        }

                    }
                }
            }
        }

        // generate black pawns and black king castling moves
        else{
            if(piece==p){
                //loop till all the bits are exhausted
                while(bitboard){
                    // init ssource square
                    source_square=get_lsb_index(bitboard);

                    //printf("white pawn: %s", square_to_coordinates[source_square]);

                    // init target square
                    target_square=source_square+8;

                    if(!(target_square>h1) && !(get_bit(occupancies[both], target_square))){

                        // for pawn promotions
                        if(source_square>=a2 && source_square<=h2){

                            // add move to the list
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));


                        }
                        // pawn moves without promotion
                        else{
                            // one move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0 , 0, 0, 0, 0));

                            // two move oush
                            if((source_square>=a7 && source_square<=h7) && !get_bit(occupancies[both], target_square+8)){
                                add_move(move_list, encode_move(source_square, target_square+8, piece, 0 , 0, 1, 0, 0));
                            }

                        }

                    }

                    // init attacks
                    attacks= pawn_attacks[side][source_square]& occupancies[white];

                    // loop
                    while(attacks){
                        //printf("yp\n");
                        // init target
                        target_square=get_lsb_index(attacks);

                        // add move to the list
                        // pawn promotion
                        if(source_square>=a2 && source_square<=h2){

                            // add move to the list
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));

                        }
                        else{
                            // add move to the list
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target_square);
                    }
                        // enpassant attack

                    if(enpassant!=no_sq){
                        // enpassant attacks
                        U64 enpassant_attacks= pawn_attacks[side][source_square] & (1ULL << enpassant);

                        //if enpassant attack available
                        if(enpassant_attacks){
                            // add move to the list
                            int target_enpassant= get_lsb_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    pop_bit(bitboard, source_square);
                }



            }
            // castling moves
            if(piece==k){
                // king side castling
                    if(castle & bk){

                        // check if square between king and rook are empty
                        if(!(get_bit(occupancies[both], f8)) && !(get_bit(occupancies[both], g8))){

                            //check if king and f1 square are not attacked by any piece
                            if(!(is_square_attacked(e8, white)) && !(is_square_attacked(f8, white)) ){
                                add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                            }

                        }
                    }
                    // queen side castling
                    if(castle & bq){
                        // check if d1 and c1 and b1 are empty
                        if(!(get_bit(occupancies[both], c8)) && !(get_bit(occupancies[both], d8)) && !(get_bit(occupancies[both], b8))){

                            //check if e1 and d1 are not attacked by any piece
                            if(!(is_square_attacked(e8, white)) && !(is_square_attacked(d8, white))){
                                add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                            }

                        }
                    }
            }

        }


        // knight move generator
        if((side==white)? piece==N: piece==n){

            // loop square bitboard of the knights
            while (bitboard){
                /* code */
                source_square=get_lsb_index(bitboard);

                // init attacks of the knights
                attacks= knight_attacks[source_square] & ((side==white)? ~occupancies[white]: ~occupancies[black]);

                // loop over atacks
                while(attacks){
                    target_square=get_lsb_index(attacks);

                    // quite move
                    if(!get_bit(((side==white)? occupancies[black]: occupancies[white]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop the bit from attacks
                    pop_bit(attacks, target_square);
                }

                // pop the bit from bitboard
                pop_bit(bitboard, source_square);
            }

        }

        // bishop move generator
        if((side==white)? piece==B: piece==b){

            // loop square bitboard of the knights
            while (bitboard){
                /* code */
                source_square=get_lsb_index(bitboard);

                // init attacks of the knights
                attacks= (get_bishop_attacks(source_square,occupancies[both])) & ((side==white)? ~occupancies[white]: ~occupancies[black]);

                // loop over atacks
                while(attacks){
                    target_square=get_lsb_index(attacks);

                    // quite move
                    if(!get_bit(((side==white)? occupancies[black]: occupancies[white]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop the bit from attacks
                    pop_bit(attacks, target_square);
                }

                // pop the bit from bitboard
                pop_bit(bitboard, source_square);
            }

        }

        // rook move generator
        if((side==white)? piece==R: piece==r){

            // loop square bitboard of the knights
            while (bitboard){
                /* code */
                source_square=get_lsb_index(bitboard);

                // init attacks of the knights
                attacks= (get_rook_attacks(source_square,occupancies[both])) & ((side==white)? ~occupancies[white]: ~occupancies[black]);

                // loop over atacks
                while(attacks){
                    target_square=get_lsb_index(attacks);

                    // quite move
                    if(!get_bit(((side==white)? occupancies[black]: occupancies[white]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop the bit from attacks
                    pop_bit(attacks, target_square);
                }

                // pop the bit from bitboard
                pop_bit(bitboard, source_square);
            }

        }

        // queen move generator

        if((side==white)? piece==Q: piece==q){

            // loop square bitboard of the knights
            while (bitboard){
                /* code */
                source_square=get_lsb_index(bitboard);

                // init attacks of the knights
                attacks= (get_queen_attacks(source_square,occupancies[both])) & ((side==white)? ~occupancies[white]: ~occupancies[black]);

                // loop over atacks
                while(attacks){
                    target_square=get_lsb_index(attacks);

                    // quite move
                    if(!get_bit(((side==white)? occupancies[black]: occupancies[white]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    // pop the bit from attacks
                    pop_bit(attacks, target_square);
                }

                // pop the bit from bitboard
                pop_bit(bitboard, source_square);
            }
        }

            // king moves

        if((side==white)? piece==K: piece==k){

                // loop square bitboard of the knights
            while (bitboard){
                 /* code */
                source_square=get_lsb_index(bitboard);

                // init attacks of the knights
                attacks= king_attacks[source_square] & ((side==white)? ~occupancies[white]: ~occupancies[black]);

                    // loop over atacks
                while(attacks){
                    target_square=get_lsb_index(attacks);

                    // quite move
                    if(!get_bit(((side==white)? occupancies[black]: occupancies[white]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop the bit from attacks
                        pop_bit(attacks, target_square);
                }

                    // pop the bit from bitboard
                    pop_bit(bitboard, source_square);
            }



        }
    }

}
/*******************

******perft  *******

********************/
// get time in milli seconds
int get_time_ms(){
    return GetTickCount();
}

// leaf nodes (no of positions reached during the test of the move generator at a given depth)
long nodes;

// perf driver
static inline void perft_driver(int depth){
    // recursion escape condition
    if(depth==0){

        // increment nodes count(count reached position)
        nodes++;
        return;
    }

    moves move_list[1];


    generate_moves(move_list);

    for(int move_count=0; move_count<move_list->count; move_count++){



            // preserve boiard state
            copy_board();

            if(!make_move(move_list->moves[move_count],all_moves)){
                continue;
            }

            perft_driver(depth-1);
            // take back
            take_back();

    }

}

// perft test
void perft_test(int depth){

    printf("\n     performance test\n\n");
    moves move_list[1];

    generate_moves(move_list);

    // init start time
    long start= get_time_ms();

    for(int move_count=0; move_count<move_list->count; move_count++){



            // preserve boiard state
        copy_board();

        if(!make_move(move_list->moves[move_count],all_moves)){
                continue;
        }

        long cumulative_nodes=nodes;

        perft_driver(depth-1);
        long old_nodes= nodes-cumulative_nodes;
        // take back

        take_back();

        // print move;
        printf("      move: %s%s%c  nodes:%ld\n",square_to_coordinates[get_move_source(move_list->moves[move_count])],
                                                square_to_coordinates[get_move_target(move_list->moves[move_count])],
                                                promoted_pieces[get_move_promoted(move_list->moves[move_count])],
                                                old_nodes);


    }

    // print results
    printf("\n     depth: %d \n",depth);
    printf("\n     nodes: %ld\n", nodes);
    printf("\n      Time: %ld\n", get_time_ms()-start);

}

/*******************

*** init all *******

********************/
/// init all function
void init_all(){
    // init leaper pieces

    init_leaper_pieces();
    init_slider_attacks(bishop);
    init_slider_attacks(rook);
    // init magic numbers
    //init_magic_numbers();
}


int material_score[12]={
    100,
    300,
    350,
    500,
    1000,
    10000,
    -100,
    -300,
    -350,
    -500,
    -1000,
    -10000

};

// pawn positional square
const int pawn_score[64]= {
    90,90,90,90,90,90,90,90,
    30,30,30,40,40,30,30,30,
    20,20,20,30,30,30,20,20,
    10,10,10,20,20,10,10,10,
    5, 5 ,10,20,20, 5, 5, 5,
    0 ,0 ,0 ,5 ,5 ,0 ,0 ,0 ,
    0 ,0 ,0 ,-10 ,-10 ,0 ,0 ,0 ,
    0 ,0 ,0 ,0 ,0 ,0 ,0 ,0

};

// knight score
const int knight_score[64] =
{
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0, 10, 10,  0,  0, -5,
    -5,  5, 20, 20, 20, 20,  5, -5,
    -5, 10, 20, 30, 30, 20, 10, -5,
    -5, 10, 20, 30, 30, 20, 10, -5,
    -5,  5, 20, 10, 10, 20,  5, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5, -10,  0,  0,  0,  0, -10, -5
};


// bishop positional score
const int bishop_score[64] =
{
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0, 10, 10,  0,  0,  0,
    0,  0, 10, 20, 20, 10,  0,  0,
    0,  0, 10, 20, 20, 10,  0,  0,
    0, 10,  0,  0,  0,  0, 10,  0,
    0, 30,  0,  0,  0,  0, 30,  0,
    0,  0, -10,  0,  0, -10,  0,  0
};

// rook positional score
const int rook_score[64] =
{
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    0,  0, 10, 20, 20, 10,  0,  0,
    0,  0, 10, 20, 20, 10,  0,  0,
    0,  0, 10, 20, 20, 10,  0,  0,
    0,  0, 10, 20, 20, 10,  0,  0,
    0,  0, 10, 20, 20, 10,  0,  0,
    0,  0,  0, 20, 20,  0,  0,  0
};

// king positional score
const int king_score[64] =
{
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  5,  5,  5,  5,  0,  0,
    0,  5,  5, 10, 10,  5,  5,  0,
    0,  5, 10, 20, 20, 10,  5,  0,
    0,  5, 10, 20, 20, 10,  5,  0,
    0,  0,  5, 10, 10,  5,  0,  0,
    0,  5,  5, -5, -5,  0,  5,  0,
    0,  0,  5,  0, -15,  0, 10,  0
};

// mirror positional score tables for opposite side
const int mirror_score[128] =
{
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};
// position evaluation
static inline int evaluate(){

    int score=0;

    U64 bitboard;

    int piece,square;

    // loop over piece bitboards
    for(int bb_piece=P;bb_piece<=k; ++bb_piece){

        bitboard=bitboards[bb_piece];

        while(bitboard){
            // init piece
            piece=bb_piece;

            //getting the  square
            square=get_lsb_index(bitboard);

            // calcu the score by material weights
            score+=material_score[piece];

            // score acco to the positiom
            switch (piece)
            {
                // evaluate the white piece
                case P: score+= pawn_score[square]; break;
                case N: score+= knight_score[square]; break;
                case B: score+= bishop_score[square]; break;
                case R: score+= rook_score[square]; break;
                case K: score+= king_score[square]; break;

                // evaluete the black pieces
                case p: score-= pawn_score[mirror_score[square]]; break;
                case n: score-= knight_score[mirror_score[square]]; break;
                case b: score-= bishop_score[mirror_score[square]]; break;
                case r: score-= rook_score[mirror_score[square]]; break;
                case k: score-= king_score[mirror_score[square]]; break;
            }
            pop_bit(bitboard,square);
        }
    }
    return (side==white)? score: -score;
}
/*****************************/
/*===========================*/

//         search

/*===========================*/
/****************************/
// mvv_lva[attackers][victim]
static int mvv_lva[12][12] = {
    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
    104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
    103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
    102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
    101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600,
    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
    104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
    103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
    102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
    101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600
};

// define max ply
#define max_ply 64

// kilkler moves[id][ply]
int killer_moves[2][max_ply];

// history moves[piece][square]
int history_move[12][64];

// pv length
int pv_length[max_ply];

// pv table
int pv_table[max_ply][max_ply];

// follow pv & score PV move
int follow_pv, score_pv;

//half counter move
int ply;

static inline void enable_pv_scoring(moves *move_list){
    // disable following ppv
    follow_pv=0;

    // loop pver the moves within the move list
    for(int count =0; count < move_list->count; ++count){
        // if the move is the pv move
        if(move_list->moves[count]==pv_table[0][ply]){
            // enable following pv
            follow_pv=1;

            // score pv move
            score_pv=1;


        }
    }
}

/*
==============================
        Move ordering
==============================

1. PV move
2. Captures in MVV/LVA
3. 1st killer move
4. 2nd killer move
5. History moves
6. Unsorted moves
*/


// score moves
static inline int score_move(int move){
    // if pv move scoring is allowed
    if(score_pv){

        if(pv_table[0][ply]==move){
            // disable core pv flag
            score_pv=0;

            // give pv search highest score
            return 20000;
        }
    }


    // score capture move
    if(get_move_capture(move)){
        //init target piece
        int target_piece=P;

        // pick up bitboard piece index ranges depending on the side
        int start_piece=(side==white)? p: P;
        int end_piece=(side==white)? k: K;

        // loop over the bitboards opposite to the current side to move
        for(int bb_piece=start_piece; bb_piece<=end_piece; ++bb_piece){
            // if the target piece is found
            if(get_bit(bitboards[bb_piece], get_move_target(move))){
                target_piece=bb_piece;
                break;
            }
        }
        // score move by mvv lva lookups [sourcepiece][targetpiece]

        return mvv_lva[get_move_peice(move)][target_piece]+10000;
    }
    else{
        // score 1st killer move
        if(killer_moves[0][ply]==move){
            return 9000;
        }
        // score 2nd killer move
        else if(killer_moves[1][ply]==move){
            return 8000;

        }
        // history moves
        else{
            return history_move[get_move_peice(move)][get_move_target(move)];
        }
    }
    return 0;
}

//sort moves in the descending order
static inline int sort_moves(moves *move_list){

    // move scores
    int move_scores[move_list->count];

    // score all the move within a move list
    for(int count=0; count<move_list->count; ++count){
        move_scores[count]=score_move(move_list->moves[count]);
    }

    // loop over current move within the list
    for(int current_move=0; current_move< move_list->count; ++current_move){
        // loop over the rest of the moves
        for(int next_moves=current_move+1; next_moves<move_list->count; ++next_moves){
            // if the current move score is less than the rest of the moves
            if(move_scores[current_move]<move_scores[next_moves]){
                // swap the scores
                int temp_score=move_scores[current_move];
                move_scores[current_move]=move_scores[next_moves];
                move_scores[next_moves]=temp_score;

                // swap the moves
                int temp_move=move_list->moves[current_move];
                move_list->moves[current_move]=move_list->moves[next_moves];
                move_list->moves[next_moves]=temp_move;

            }
        }
    }

}
// print move scores
void print_move_scores(moves *move_list){
    // loop over the moves in the move list
    printf("  move scores:\n\n");
    for(int count=0; count<move_list->count; ++count){
        printf("  move: ");
        print_move(move_list->moves[count]);
        printf(" score: %d\n", score_move(move_list->moves[count]));
    }
}
// quiesence search
static inline int quiescence(int alpha, int beta){

    // increment nodes;
    ++nodes;

    // evaluate position
    int evaluation= evaluate();

    if(evaluation>=beta){
            // node move fail high
            return beta;
    }
    if(evaluation>alpha){
            alpha=evaluation;
    }
    moves move_list[1];

    //generate moves
    generate_moves(move_list);

    // sort moves
    sort_moves(move_list);

    // loop over the moves in the move list
    for(int count=0; count< move_list->count; ++count){
        // preserve board state
        copy_board();
        ++ply;

        // make sure it makes a leagl move
        if(make_move(move_list->moves[count],only_captures)==0){

            --ply;
            continue;
        }


        // score current mmoevb
        int score= -quiescence(-beta,-alpha);

        --ply;
        // take back
        take_back();

        if(score>=beta){
            // node move fail high
            return beta;
        }
        if(score>alpha){
            alpha=score;
        }
    }

    return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;


//negamx alpha beta search
static inline int negamax(int alpha, int beta, int depth){

    // init pv length
    pv_length[ply]=ply;

    if(depth==0){
        //return quiescense search
        return quiescence(alpha,beta);
    }

    // we are too deep that will led to overflows of arrays relying on the max_ply
    if(ply>max_ply-1){
        return evaluate();
    }

    // incerement node cnt
    ++nodes;

    // is kimng in check
    int in_check= is_square_attacked((side==white)? get_lsb_index(bitboards[K]): get_lsb_index(bitboards[k]), side^1);

    if(in_check){
        ++depth;
    }
    // legal moves counter
    int legal_moves=0;;

    // null move pruning
    if(depth>=3 && in_check==0 && ply){

        // preserve board state
        copy_board();

        // switch the side, literally giving opponent an extra move to make
        side^=1;

        //reset enpassant square
        enpassant=no_sq;

        // search mives with reduced depth to find beta cutoffs
        // depth-1-R where R is the reduction value which is 2
        int score= -negamax(-beta, -beta+1, depth-1-2);

        // restore board state
        take_back();

        // fail hard beta cutoofr
        if(score>=beta){
            // node (move) fails high
            return beta;
        }

    }

    moves move_list[1];

    //generate moves
    generate_moves(move_list);

    // if we are followinf pv line
    if(follow_pv){
        // enable pv move scoring
        enable_pv_scoring(move_list);
    }
    // sort moves
    sort_moves(move_list);

    // no of moves searched in amove list
    int move_searched=0;

    // loop over the moves in the move list
    for(int count=0; count< move_list->count; ++count){
        // preserve board state
        copy_board();
        ++ply;

        // make sure it makes a leagl move
        if(make_move(move_list->moves[count],all_moves)==0){

            --ply;
            continue;
        }

        // increment the legal moves
        ++legal_moves;

        /*
        if (fFoundPv) {
            val = -AlphaBeta(depth - 1, -alpha - 1, -alpha);

            if ((val > alpha) && (val < beta)) // Check for failure.

                val = -AlphaBeta(depth - 1, -beta, -alpha);

        } else

            val = -AlphaBeta(depth - 1, -beta, -alpha);
        */

        int score=0;
        //full depth search
        if(move_searched==0){
            score = -negamax(-beta, -alpha, depth-1);
        }

        // elsse late move reduction
        else{
            //condition to consider lmr
            if(
                move_searched >= full_depth_moves &&
                depth >= reduction_limit &&
                in_check == 0 &&
                get_move_capture(move_list->moves[count]) == 0 &&
                get_move_promoted(move_list->moves[count]) == 0
                )
            {
                // search current move with reduced depth:
                score = -negamax(-alpha - 1, -alpha, depth - 2);
            }
                // hack to ensure that full-depth search is done
            else{
                 score = alpha + 1;
            }
                // principal variation search(PVS)
            if(score > alpha)
            {
                /* Once you've found a move with a score that is between alpha and beta,
                the rest of the moves are searched with the goal of proving that they are all bad.
                It's possible to do this a bit faster than a search that worries that one
                of the remaining moves might be good. */
                score = -negamax(-alpha - 1, -alpha, depth-1);

                /* If the algorithm finds out that it was wrong, and that one of the
                subsequent moves was better than the first PV move, it has to search again,
                in the normal alpha-beta manner. This happens sometimes, and it's a waste of time,
                but generally not often enough to counteract the savings gained from doing the
                "bad move proof" search referred to earlier. */
                if((score > alpha) && (score < beta)){
                    // re-search the move that has failed to be proved to be bad
                    //with normal alpha beta score bounds
                    score = -negamax(-beta, -alpha, depth-1);
                }
            }


        }

        --ply;
        // take back
        take_back();

        // increment the moves searched
        ++move_searched;

        if(score>=beta){

            // store killer moves
            if(get_move_capture(move_list->moves[count])==0){
                killer_moves[1][ply] = killer_moves[0][ply];
                killer_moves[0][ply] = move_list->moves[count];
            }
            // node move fail high
            return beta;
        }
        if(score>alpha){
            // history moves
            if(get_move_capture(move_list->moves[count])==0){
              history_move[get_move_peice(move_list->moves[count])][get_move_target(move_list->moves[count])]+=depth;
            }
            alpha=score;



            // write pv move
            pv_table[ply][ply]=move_list->moves[count];

            // loop over the next ply
            for(int next_ply=ply+1; next_ply<pv_length[ply+1]; ++next_ply){
                // copy move from deep ply into a current ply line
                pv_table[ply][next_ply]=pv_table[ply+1][next_ply];
            }

            //adjust pv length
            pv_length[ply]=pv_length[ply+1];

        }
    }

    // we donn't have any egal moves i the current position
    if(legal_moves  == 0){
        // king is in check
        if(in_check){
            // ply for finding the shotest path for a checkmate
            return -49000+ply;
        }

        //king is not in check
        else{
            // return stalemate score;
            return 0;
        }
    }

    // node move fails low
    return alpha;

}
// search position for the best move
void search_position(int depth){
    // define score
    int score=0;

    // reset nodes
    nodes=0;

    // reset follow pv flags
    follow_pv=0;
    score_pv=0;

    // clear helper data structure for search
    memset(killer_moves,0,sizeof(killer_moves));
    memset(history_move,0,sizeof(history_move));
    memset(pv_table,0,sizeof(pv_table));
    memset(pv_length,0,sizeof(pv_length));

    // define initial alpha beta bound
    int alpha=-50000;
    int beta = 50000;
    // iterative deepening
    for(int current_depth=1; current_depth<=depth; ++current_depth){

        // enable follow pv flag
        follow_pv=1;
        // find the best move in the given position
        score = negamax(alpha,beta, current_depth);

        // we fell outside the window so try again with a full width window (and the same depth)
        if(score<=alpha || score>= beta){
            alpha=-50000;
            beta=50000;
            continue;
        }

        // set up the window for the next search
        alpha=score-50;
        beta=score+50;

        printf("info score cp %d depth %d nodes %ld pv ",score,current_depth,nodes);

        // loop over the moves within a pv line
        for(int count =0;count<pv_length[0];++count){
            print_move(pv_table[0][count]);
            printf(" ");
        }
        printf("\n");

    }
    printf("bestmove ");
    print_move(pv_table[0][0]);
    printf("\n");




}


/*****************************/
/*===========================*/

//            UCI

/*===========================*/
/****************************/

// parse user/gui string input  (e7e8Q)
int parse_move(char *move_string){
    // create move list instance
    moves move_list[1];

    // genertae moves
    generate_moves(move_list);

    // parse source square
    int source_square= (move_string[0]-'a')+(8-(move_string[1]-'0'))*8;

    // parse target square
    int target_square=(move_string[2]-'a')+(8-(move_string[3]-'0'))*8;

    // loop over the moves in the move list
    for(int move_count=0; move_count<move_list->count; ++move_count){

        // init move;
        int move= move_list->moves[move_count];

        // make sure source aquare and the target square are available
        if(source_square==get_move_source(move) && target_square==get_move_target(move)){

            // if promotion
            int promoted_piece=get_move_promoted(move);
            // promoted piece is available
            if(promoted_piece){

                // promoted to queen
                if((promoted_piece==Q || promoted_piece==q)  && move_string[4]=='q'){
                    return move;
                }

                // promoted to rook
                else if((promoted_piece==R || promoted_piece==r)  && move_string[4]=='r'){
                    return move;
                }

                // promoted to bishop
                else if((promoted_piece==B || promoted_piece==b)  && move_string[4]=='b'){
                    return move;
                }

                // promoted to knight
                else if((promoted_piece==N || promoted_piece==n)  && move_string[4]=='n'){
                    return move;
                }

                // if not a valid promotion
                continue;
            }

            return move;
            // return legal move
            //return 1;
        }
    }

    // return illegal move
    return 0;
}

// parse UCI *position command
void parse_position(char *command){
    // shift pointer to the right where next token begins
    command+=9;

    // init pointer to the current character in the command string
    char *current_char = command;

    // parse UCI "startpos" command
    if(strncmp(command,"startpos",8)==0){
        // init chess board with start position
        parse_fen(start_position);
    }
    // parse UCI "fen"  command
    else{

        // mke sure "fen" command is available within command string
        current_char=strstr(command,"fen");

        // if no fen command is available within the command string
        if(current_char==NULL){
            // init start position
            parse_fen(start_position);
        }

        // found "fen substring
        else{
            // shift pointer to the right
            current_char+= 4;

            // init chess board with the position from fen string
            parse_fen(current_char);
            // parse fen current char

        }
        printf("%s\n",current_char);
    }

    // parse moves after the position
    current_char=strstr(command,"moves");
    if(current_char!=NULL){
        // shift pt to +6'
        current_char+= 6;

        // loop over the loop s in the move string
        while(*current_char){

            // parse next move
            int move=parse_move(current_char);

            // if no more move
            if(move==0){
                break;
            }

            // make move on th chess board
            make_move(move,all_moves);

            // move current character pointer to the end of the current move
            while(*current_char && *current_char != ' '){
                current_char++;
            }

            // go to the next omve
            current_char++;
        }

    }
    // porint board
    print_board();
}


// parse uci go command
void parse_go(char *command){

    // init depth
    int depth=-1;

    //init character  pointer to the current depth argument
    char *current_depth=NULL;

    // handle fixed depth search
    if(current_depth=strstr(command,"depth")){
        // convert string  to integer and assign the result value to depth
        depth=atoi(current_depth+6);
    }

    else{
        depth=6;

    }

    // search position
     search_position(depth);
}

/// main uci loop
void uci_loop(){
    printf("BBC chess engine\n");
    // reset stdin and stdout buffers
    setbuf(stdin,NULL);
    setbuf(stdout,NULL);

    // define user and gui input buffer
    char input[2000];

    // print engine info
    printf("id name BBC\n");
    printf("id name ghost\n");
    printf("uciok\n");

    //main loop
    while(1){

        //reset user/gui input
        memset(input,0,sizeof(input));

        // make sure the output reaches the gui
        fflush(stdout);

        //get user /gui input
        if(!fgets(input,2000,stdin)){
            // continue the loop
            continue;
        }

        // make sure the input is available
        if(input[0]== '\n'){
            //continue the loop
            continue;
        }

        // parse uci "isready" command
        if(strncmp(input,"isready",7) == 0){
            printf("readyok\n");
            continue;
        }

        // parse uci "position" command
        else if(strncmp(input,"position",8)==0){
            parse_position(input);

        }
        // parse uci "ucinewgame" command
        else if(strncmp(input,"ucinewgame",10)==0){
            //call parse position function
            parse_position("position startpos");
        }
        // pass uci go command
        else if(strncmp(input,"go",2)==0){
            parse_go(input);
        }
        // pass uci quit command
        else if(strncmp(input,"quit",4)==0){
            break;
        }

        //PARSE UCI "uci" command
        else if(strncmp(input,"uci",3)==0){
            printf("id name BBC\n");
            printf("id author ghost\n");
            printf("uciok\n");
        }

    }
}


int main(){
    init_all();



    int debug=0;

    if(debug){
        parse_fen(tricky_position);
        print_board();
        search_position(8);

    }

    else{
        uci_loop();
    }



    return 0;
}