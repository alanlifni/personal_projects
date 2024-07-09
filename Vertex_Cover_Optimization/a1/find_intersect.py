def find_intersect (coordinates):
    #[[x1,y1],[x2,y2]],[[x3,y3],[x4,y4]]
    x1, y1 = coordinates[0][0][0], coordinates[0][0][1]
    x2, y2 = coordinates[0][1][0], coordinates[0][1][1]
    x3, y3 = coordinates[1][0][0], coordinates[1][0][1]
    x4, y4 = coordinates[1][1][0], coordinates[1][1][1]
 
    try:
        m1 = (y1-y2)/(x1-x2)  # Pay attention to not dividing by zero
        m1 = round(m1,4)
    except:
        m1  = 'Nan'

    try:
        m2 = (y3-y4)/(x3-x4)  # Pay attention to not dividing by zero
        m2 = round(m2,4)
    except:
        m2 = 'Nan'

    if (m1 == m2):
        return [False, None, None]  # Parallel segments
    
    xnum = ((x1*y2-y1*x2)*(x3-x4) - (x1-x2)*(x3*y4-y3*x4))
    xden = ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4))
    xcoor = round( xnum / xden, 2)

    ynum = (x1*y2 - y1*x2)*(y3-y4) - (y1-y2)*(x3*y4-y3*x4)
    yden = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4)
    ycoor = round( ynum / yden, 2)
    
    if ((((xcoor >= x1 and xcoor <= x2) or (xcoor <= x1 and xcoor >= x2)) and ((xcoor >= x3 and xcoor <= x4) or (xcoor <= x3 and xcoor >= x4))) and
       (((ycoor >= y1 and ycoor <= y2) or (ycoor <= y1 and ycoor >= y2)) and ((ycoor >= y3 and ycoor <= y4) or (ycoor <= y3 and ycoor >= y4)))):
        intersection = True;
    else:
        return [False, None, None]  # intersection is out of bounds
    
    return [intersection, xcoor, ycoor]   