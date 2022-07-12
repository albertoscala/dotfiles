# i: indice delle y
# j: indice delle x
# s: numero di svolte
# d: direzione (0 sulle x; 1 sulle y)
def contaTragitti(n, s):
    i = j = n - 1

    if i == 0 and j == 0:
        return 1

    tab = [[[[-1 for _ in range(2)]
             for _ in range(s + 1)]
            for _ in range(n)]
           for _ in range(n)]

    # Primo caso base
    # i == 0 && j == 0
    for sv in range(s + 1):
        for d in range(2):
            tab[0][0][sv][d] = 1

    # Secondo caso base

    # d == 0 && i == 0
    for j in range(n):
        for sv in range(s + 1):
            tab[0][j][sv][0] = 1

    # d == 1 && j == 0
    for i in range(n):
        for sv in range(s + 1):
            tab[i][0][sv][1] = 1

    # caso base tabella s = 0 && d = 0
    for i in range(1, n):
        for j in range(n):
            tab[i][j][0][0] = 0

    # caso base tabella s = 0 && d = 1
    for j in range(1, n):
        for i in range(n):
            tab[i][j][0][1] = 0

    for sv in range(s + 1):
        for d in range(2):
            for i in range(n):
                for j in range(n):
                    print(tab[i][j][0][0], end='')
                print()
                
    for sv in range(1, s + 1):
        for i in range(1, n):
            for j in range(1, n):
                # d = 0
                # print("tab(" + str(i) + "," + str(j) + "," + str(sv) + "," + str(0) + ") = " + str(tab[i - 1][j][sv][0]) + " + " +
                #      str(tab[i][j - 1][sv - 1][1]))
                tab[i][j][sv][0] = tab[i - 1][j][sv][0] + tab[i][j - 1][sv - 1][1]

                # d = 1
                # print("tab(" + str(i) + "," + str(j) + "," + str(sv) + "," + str(1) + ") = " + str(tab[i - 1][j][sv - 1][0]) + " + " +
                #      str(tab[i][j - 1][sv][1]))
                tab[i][j][sv][1] = tab[i - 1][j][sv - 1][0] + tab[i][j - 1][sv][1]

    return tab[n - 1][n - 1][s][1]


if __name__ == '__main__':
    n = int(input("Dimensione matrice: "))
    s = int(input("Max svolte da fare: "))
    print("Totale percorsi: " + str(contaTragitti(n, s)))
