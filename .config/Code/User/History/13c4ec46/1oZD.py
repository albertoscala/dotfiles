# i: indice delle y
# j: indice delle x
# s: numero di svolte
# d: direzione (0 sulle x; 1 sulle y)
def contaTragittiSvolte(i, j, s, d, memo):
    if i < 0 or j < 0:
        return 0

    if i == 0 and j == 0:
        return 1

    if s == 0:
        if d == 0 and i == 0:
            return 1

        if d == 1 and j == 0:
            return 1

        return 0

    if memo[i][j][s][d] is not None:
        return memo[i][j][s][d]

    if d == 1:
        memo[i][j][s][d] = contaTragittiSvolte(i - 1, j, s, 1, memo) + contaTragittiSvolte(i, j - 1, s - 1, 0, memo)
        print(memo[i][j][s][d])
        return memo[i][j][s][d]
    else:
        memo[i][j][s][d] = contaTragittiSvolte(i - 1, j, s - 1, 1, memo) + contaTragittiSvolte(i, j - 1, s, 0, memo)
        print(memo[i][j][s][d])
        return memo[i][j][s][d]


def contaTragitti(n, s):

    i = j = n - 1

    if i == 0 and j == 0:
        return 1

    memo = [[[[None for w in range(2)]
            for z in range(s + 1)]
            for y in range(j + 1)]
            for x in range(i + 1)]

    return contaTragittiSvolte(i - 1, j, s, 0, memo) + contaTragittiSvolte(i, j - 1, s, 1, memo)


if __name__ == '__main__':
    n = int(input("Dimensione matrice: "))
    s = int(input("Max svolte da fare: "))
    print("Totale percorsi: " + str(contaTragitti(n, s)))